#include "serverFunctions.h"
#include<sys/stat.h>
#include<pthread.h>
#include<time.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define TABLE_TEMPLATE "%-16s\t\t"
#define MAX_COL 20
#define QUERY_BUFFER_SIZE 1024

static FILE *log;
ServerArguments serverArguments;
Node *columnNamesHead;
Node **rowsHeads;
int numberOfColumn=0;
int numberOfRow=0;

int sfd;
int AR=0,AW=0,WR=0,WW=0;
int activeThread=0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t okToRead = PTHREAD_COND_INITIALIZER; 
pthread_cond_t okToWrite = PTHREAD_COND_INITIALIZER; 
pthread_cond_t wait = PTHREAD_COND_INITIALIZER; 

int becomeDaemon(int argc, char **argv){
    pid_t pid;

    pid = fork();
    if(pid == 0) {
        initialErrorCheck(argc, argv);
        server_handleCommandline(argc,argv);
        server_readCSV();
        server_openSocket();
        server_freeColumns();
    }else{
        exit(EXIT_SUCCESS);
    }

    umask(0);

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}

void catchSIGINT(int sigNum){
    fprintf(log,"Termination signal received, waiting for ongoing threads to complete.\n");
    fprintf(log,"All threads have terminated, server shutting down.\n");
    fflush(log);

    server_freeColumns();
}

void initialErrorCheck(int argc,char **argv){
    if(argc != 9){
        printErrorExit("Argument Error: Insufficient argument.\n");
    }
}

void server_handleCommandline(int argc,char **argv){
    int opt=0;
    int pflag=0, oflag=0, lflag=0, dflag=0;

    while((opt = getopt(argc, argv, "p:o:l:d:")) != -1) {  
        switch(opt) { 
            case 'p': 
                pflag++;
                if(isPositiveNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -p flag's argument has to be number.\n");
                }
                strcpy(serverArguments.port,optarg);
                break;
            case 'o': 
                oflag++;
                strcpy(serverArguments.pathToLogFile,optarg);
                break;
            case 'l':
                lflag++;
                if(isPositiveNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -l flag's argument has to equal or greater than 2.\n");
                }
                serverArguments.poolSize = atoi(optarg);
                if(serverArguments.poolSize < 2){
                    printErrorExit("Wrong parameter argument: -l flag's argument has to equal or greater than 2.\n");
                }
                break;
            case 'd':
                dflag++;
                strcpy(serverArguments.datasetPath,optarg);
                break;
        }
    }

    if(pflag > 1 || oflag > 1 || lflag > 1 || dflag > 1){
        printErrorExit("Each flag can only use one time.(Flags : -p, -o, -l, -d) \n");
    }

    if(pflag == 0 || oflag == 0 || lflag == 0 || dflag == 0){
        printErrorExit("Each flag can only use one time.(Flags : -p, -o, -l, -d) \n");
    }

    log = fopen(serverArguments.pathToLogFile,"w");
}

void server_readCSV(){
    int i=0,row=0,delNum=0,flag=0;
    char *token,*rest=NULL;
    char line[512];
    
    clock_t time = clock();

    FILE *fp = fopen(serverArguments.datasetPath,"r");
    if(fp == NULL){
        printErrorExit("CSV File Error.\n");
    }

    while(fgets(line,sizeof(line),fp)){
        numberOfRow++;
        line[strlen(line)-2] = '\0';
        //printf("%s",line);
        
        delNum = removeCommasInDelimiter(line,'"',0);
        delNum = removeCommasInDelimiter(line,'"',delNum);
        delNum = removeCommasInDelimiter(line,'"',delNum);

        //printf("%s",line);

        for(token=strtok_r(line,",",&rest),row=0 ; token!=NULL ; token=strtok_r(NULL,",",&rest),row++){
            if(i==0){
                //printf("%s\n",token);
                push(&columnNamesHead,token);
                //strcpy(columnNames[numberOfColumn],token);
                numberOfColumn++;
            } else{
                if(flag==0){
                    rowsHeads = (Node **) calloc(numberOfColumn,sizeof(Node*));
                    flag = 1;
                }
              
                if(row >= numberOfColumn){
                    printErrorExit("row >= numberOfColumn\n");
                    break;
                }

                //printf("%s\n",token);
                push(&rowsHeads[row % numberOfColumn],token);
            }
        }
        //printf("\n");
        ++i;
    }

    fclose(fp);

    time = clock() - time;

    //printf("->Number Of Column: %d\n",numberOfColumn);
    //printf("->Number Of Row: %d\n",numberOfRow);

    server_output(time, numberOfRow);
}

void server_output(double time, long data){
    fprintf(log,"Executing with parameters:\n");
    fprintf(log,"-p %s\n",serverArguments.port);
    fprintf(log,"-o %s\n",serverArguments.pathToLogFile);
    fprintf(log,"-l %d\n",serverArguments.poolSize);
    fprintf(log,"-d %s\n",serverArguments.datasetPath);
    fprintf(log,"Loading dataset...\n");
    fprintf(log,"Dataset loaded in %.2f seconds with %ld records.\n",(double)time/CLOCKS_PER_SEC,data);
    fflush(log);
}

void server_openSocket(){
    struct sockaddr_in addr;

    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0)
        printErrorExit("Socket Error.\n");

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(serverArguments.port));

    if(bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        printErrorExit("Bind Error.\n");

    if(listen(sfd, 5) < 0)
        printErrorExit("Listen Error.\n");

    fprintf(log,"A pool of %d threads has been created\n",serverArguments.poolSize);
    server_createPoolThreads();
}

void server_createPoolThreads(){
    int i=0;
    pthread_t poolThreads[serverArguments.poolSize];

    for(i=0; i<serverArguments.poolSize; i++){
        int *m = malloc(sizeof(int*));
        *m = i;
        pthread_create(&poolThreads[i], NULL, threadFunction, (void *) m);
    }

    for(i=0; i<serverArguments.poolSize; i++){
        pthread_join(poolThreads[i], NULL);
    }
}

void *threadFunction(void *arg){
    int cfd;
    struct sockaddr_in cli;
    char buffer[QUERY_BUFFER_SIZE] = {0};
    int clilen = sizeof(cli);
    int count, totalRecords;
    Node *readerColumnsHead;
    Node **readerRowsHead;
    Message message;

    int num = *(int*) arg;

    fprintf(log,"Thread #%d: waiting for connection\n", num);
    fflush(log);

    for(;;){
        cfd = accept(sfd, (struct sockaddr *)&cli, (socklen_t *)&clilen);
        if(cfd < 0){
            printErrorExit("Cfd Error.\n");
        }   

        pthread_mutex_lock(&lock2);
        if(activeThread == serverArguments.poolSize){
            pthread_cond_wait(&wait,&lock2);
        }
        pthread_mutex_unlock(&lock2);

        read(cfd,buffer,QUERY_BUFFER_SIZE);

        activeThread += 1;

        fprintf(log,"A connection has been delegated to thread id #%d\n", num);
        fprintf(log,"Thread #%d: received query '%s'\n", num, buffer);
        fflush(log);

        Query _query;
        _query.isAllColumn = 0;
        _query.isDistinct = 0;
        _query.selectedLength = 0;

        server_parseQuery(buffer, &_query);

        clock_t time = clock();

        if(_query.type == SELECT){
            server_reader(&_query, &readerColumnsHead, &readerRowsHead, &count, &totalRecords);
        } else if(_query.type == UPDATE){
            server_writer(&_query,&message);
        }

        time = clock() - time;

        //printf("%f \n",(double)time/CLOCKS_PER_SEC);
        if(_query.type == SELECT)
            server_createAMessage(readerColumnsHead,readerRowsHead,&message,count,totalRecords,time,_query.isAllColumn);

        send(cfd, (void *)&message, sizeof(message), 0);

        fprintf(log,"Thread #%d: query completed, %d records have been returned.\n", num, message.records);
        fflush(log);

        strcpy(message.buffer,"");

        sleep(0.5);
        activeThread -= 1;
    }

    return NULL;
}

void server_createAMessage(Node *readerColumnsHead, Node **readerRowsHead, Message *message, int count, int totalRecords, long time, bool isAll){
    int i=0,j=0,flag=0;
    char *tok;
    char temp[100];

    strcat(message->buffer,"\n");
    for(int i=0;i<count;++i){
        tok = AtIndex(readerColumnsHead,i);
        if(tok != NULL){
            if(isAll == true){
                sprintf(temp,TABLE_TEMPLATE, tok);
            } else{
                tok = AtIndex(readerColumnsHead,count-i-1);
                sprintf(temp,TABLE_TEMPLATE, tok);
            }
        } else{
            sprintf(temp,TABLE_TEMPLATE, " ");
        }
        strcat(message->buffer,temp);
    }
    strcat(message->buffer,"\n");
    for(j=0;j<MAX_COL;++j){
        flag = 0;
        for(i=0;i<count;++i){
            tok = AtIndex(readerRowsHead[i],j);
            if(tok != NULL){
                sprintf(temp,TABLE_TEMPLATE, tok);
            } else{
                flag++;
                sprintf(temp,TABLE_TEMPLATE, " ");
            }
            strcat(message->buffer,temp);
        }
        if(flag == count){
            break;
        }
        strcat(message->buffer,"\n");
    }
    sprintf(temp,"Maximum Number of Columns Shown : %d\n",MAX_COL);
    strcat(message->buffer,temp);

    message->records = totalRecords;
    message->seconds = (double)time/CLOCKS_PER_SEC;
}

void server_reader(const Query *query, Node **readerColumnsHead, Node ***readerRowsHead, int *count, int *totalRecords){
    pthread_mutex_lock(&lock);
    while((AW+WW) > 0){
        WR++;
        pthread_cond_wait(&okToRead,&lock);
        WR--;
    }

    AR++;
    pthread_mutex_unlock(&lock);

    //TODO: Access Database

    *count = server_readerAccessDB(query,&(*readerColumnsHead),&(*readerRowsHead),&(*totalRecords));

    pthread_mutex_lock(&lock);
    AR--;
    if(AR == 0 && WW > 0){
        pthread_cond_signal(&okToWrite);
    }
    pthread_mutex_unlock(&lock);
}

void server_writer(const Query *query, Message *message){
    int totalRecords=0,val=0;
    char temp[100];

    pthread_mutex_lock(&lock);
    while((AW+AR) > 0){
        WW++;
        pthread_cond_wait(&okToWrite,&lock);
        WW--;
    }

    AW++;
    pthread_mutex_unlock(&lock);

    val = server_updateDB(query,&totalRecords,&(*message));

    if(val == 1){
        sprintf(temp,"Updating is completed successfully.\nMaximum Number of Columns Shown : %d\n",MAX_COL);
    } else{
        sprintf(temp,"Updating is not completed successfully because of some reasons.\n");
    }
    strcat(message->buffer,temp);

    pthread_mutex_lock(&lock);
    AW--;
    if(WW > 0){
        pthread_cond_signal(&okToWrite);
    } else if (WR > 0){
        pthread_cond_broadcast(&okToRead);
    }
    pthread_mutex_unlock(&lock);
}

int server_readerAccessDB(const Query *query, Node **readerColumnsHead, Node ***readerRowsHead, int *totalRecords){
    int i=0,j=0,k=0;    
    char *tok,*tok2;
    char value[300],value2[100];
    int rowCount=0,flag=1,returnVal=0;

    if(query->isAllColumn == true){
        (*readerRowsHead) = (Node **) calloc(numberOfColumn,sizeof(Node*));

        for(j=0;j<numberOfColumn;++j,++returnVal){
            tok2 = AtIndex(columnNamesHead,j);
            if(tok2 == NULL)
                break;
            strcpy(value2,tok2);
            push(&(*readerColumnsHead),value2);

            rowCount=0;
            for(k=0;k<numberOfRow;++k){
                tok = AtIndex(rowsHeads[j],k);
                if(tok != NULL){
                    strcpy(value,tok);

                    if(query->isDistinct == true){
                        flag = findForDistinct((*readerRowsHead)[j],rowCount,tok);
                    }

                    if(flag == 1){
                        if(*totalRecords < numberOfRow)
                            *totalRecords += 1;

                        push(&(*readerRowsHead)[j],value);
                        rowCount++;
                    }
                }
            }
        }
    } else{
        (*readerRowsHead) = (Node **) calloc(query->selectedLength,sizeof(Node*));

        for(j=0;j<numberOfColumn;++j){
            tok2 = AtIndexReverse(columnNamesHead,j);
            if(tok2 == NULL)
                break;
            for(i=0;i<query->selectedLength;++i){
                if(strcmp(tok2,query->selectedColumns[i])==0){
                    strcpy(value2,tok2);
                    push(&(*readerColumnsHead),value2);

                    for(k=0;k<numberOfRow;++k){
                        tok = AtIndex(rowsHeads[j],k);
                        if(tok != NULL){
                            strcpy(value,tok);
                            if(query->isDistinct == true){
                                flag = findForDistinct((*readerRowsHead)[returnVal],rowCount,value);
                            }

                            if(flag == 1){
                                if(*totalRecords < numberOfRow)
                                    *totalRecords += 1;

                                push(&(*readerRowsHead)[returnVal],value);
                                rowCount++;
                            }
                        }
                    }
                    returnVal++;
                }
            }
        }
    }

    return returnVal;
}

int server_updateDB(const Query *query, int *totalRecords, Message *message){
    int i=0,j=0,k=0;
    char *tok,temp[100];
    int indexes[10000],colIndexes[20],len=0,colLen,updatedColLen=0;

    for(i=0;i<numberOfColumn;++i){
        if(strcmp(AtIndexReverse(columnNamesHead,i),query->whereColumn) == 0){
            updatedColLen++;
            for(j=0;j<numberOfRow;++j){
                tok = AtIndex(rowsHeads[i],j);
                if(tok != NULL){
                    if(strcmp(tok,query->whereValue)==0){
                        indexes[len++] = j;
                    }
                }
            }
        }
    }

    int val=0;

    for(i=0;i<query->selectedLength;++i){
        for(j=0;j<numberOfColumn;++j){
            if(strcmp(AtIndexReverse(columnNamesHead,j),query->selectedColumns[i]) == 0){
                sprintf(temp,TABLE_TEMPLATE, query->selectedColumns[i]);
                strcat(message->buffer,temp);

                val = updateValuesAtIndexes(rowsHeads[j],indexes,len,query->updatedColumns[i]);
                colIndexes[colLen++] = j;
            }
        }
    }

    strcat(message->buffer,"\n");
    for(k=0;k<len && k<MAX_COL;++k){
        for(i=0;i<colLen;++i){
            tok = AtIndex(rowsHeads[colIndexes[i]],indexes[k]);
            //printf("%s\n",tok);
            if(tok != NULL)
                sprintf(temp,TABLE_TEMPLATE, tok);
            else
                sprintf(temp,TABLE_TEMPLATE, " ");
            strcat(message->buffer,temp);
        }
        strcat(message->buffer,"\n");
    }
    //printf("\n%s\n",AtIndexReverse(rowsHeads[0],5));

    *totalRecords = updatedColLen * len;

    return val;
}

void server_parseQuery(char *query, Query *_query){
    int i=0,j=0,flag=0,wordsLength=2,selectedColumnNumber=0;
    char *token,*token2,*rest=NULL,*rest2=NULL;
    char *words[20] = {"FROM","WHERE"};

    for(token=strtok_r(query," ",&rest),i=0 ; token!=NULL ; token=strtok_r(NULL," ",&rest),i++){
        if(i == 0){
            if(strcmp("SELECT",token) == 0 || strcmp("Select",token) == 0 || strcmp("select",token) == 0){
                _query->type = SELECT;
            } else if (strcmp("UPDATE",token) == 0 || strcmp("Update",token) == 0 || strcmp("update",token) == 0){
                _query->type = UPDATE;
            } else{
                printErrorExit("Token Error: It must be SELECT or UPDATE.\n");
            }
        } else{ 
            if(i==1 && (strcmp("*",token) == 0)){
                _query->isAllColumn = true;
                continue;
            } else if(i==1 && (strcmp("DISTINCT",token) == 0 || strcmp("Distinct",token) == 0 || strcmp("distinct",token) == 0)){
                _query->isDistinct = true;
                continue;
            }
            if(i==2 && (strcmp("*",token) == 0)){
                _query->isAllColumn = true;
                continue;
            }
            for(j=0;j<wordsLength;++j){
                if(strcmp(words[j],token) == 0)
                    flag = 1;
            }
            if(flag == 0){
                if(_query->type == SELECT){
                    for(token2=strtok_r(token,"='’,\"",&rest2) ; token2!=NULL ; token2=strtok_r(NULL,"='’,\"",&rest2)){
                        strcpy(_query->selectedColumns[selectedColumnNumber++],token2);
                    }
                } else if(_query->type == UPDATE && i>2){
                    for(token2=strtok_r(token,"='’,\"",&rest2),j=0 ; token2!=NULL ; token2=strtok_r(NULL,"='’,\"",&rest2),j++){
                        if(j==0)
                            strcpy(_query->selectedColumns[selectedColumnNumber],token2);
                        else if(j==1)
                            strcpy(_query->updatedColumns[selectedColumnNumber],token2);
                    }  
                    selectedColumnNumber++;
                }
            } else {
                if(_query->type == UPDATE){
                    if(strcmp("WHERE",token) != 0 && strcmp("Where",token) != 0 && strcmp("where",token) != 0){
                        for(token2=strtok_r(token,"='’,\"",&rest2),j=0 ; token2!=NULL ; token2=strtok_r(NULL,"='’,\"",&rest2),j++){
                            if(j==0)
                                strcpy(_query->whereColumn,token2);
                            else if(j==1){
                                strcpy(_query->whereValue,token2);
                                if(_query->whereValue[strlen(_query->whereValue)-1] == '\n'){
                                    _query->whereValue[strlen(_query->whereValue)-1] = '\0';
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    _query->selectedLength = selectedColumnNumber;
}

void server_freeColumns(){
    int i=0;

    for(i=0;i<numberOfColumn;++i){
        freeList(rowsHeads[i]);
    }
    free(rowsHeads);
    freeList(columnNamesHead);
    fclose(log);
}