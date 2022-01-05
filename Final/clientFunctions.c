#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pthread.h>
#include<time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include "clientFunctions.h"
#include "helper.h"

ClientArguments clientArguments;
int sfd;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void initialErrorCheck(int argc,char **argv){
    if(argc != 9){
        printErrorExit("Argument Error: Insufficient argument.\n");
    }
}

void client_handleCommandline(int argc,char **argv){
    int opt=0;
    int iflag=0, aflag=0, pflag=0, oflag=0;

    while((opt = getopt(argc, argv, "i:a:p:o:")) != -1) {  
        switch(opt) { 
            case 'i': 
                iflag++;
                if(isPositiveNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -i flag's argument has to be number.\n");
                }
                clientArguments.id = atoi(optarg);
                break;
            case 'a': 
                aflag++;
                strcpy(clientArguments.ipv4Address,optarg);
                break;
            case 'p':
                pflag++;
                if(isPositiveNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -p flag's argument has to be number.\n");
                }
                strcpy(clientArguments.port,optarg);
                break;
            case 'o':
                oflag++;
                strcpy(clientArguments.pathToQueryFile,optarg);
                break;
        }
    }

    if(iflag > 1 || aflag > 1 || pflag > 1 || oflag > 1){
        printErrorExit("Each flag can only use one time.(Flags : -i, -a, -p, -o) \n");
    }

    if(iflag == 0 || aflag == 0 || pflag == 0 || oflag == 0){
        printErrorExit("Each flag can only use one time.(Flags : -i, -a, -p, -o) \n");
    }
}

void client_openSocket(){
    struct sockaddr_in addr;
    char output[250];

    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0)
        printErrorExit("Socket Error.\n");

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(clientArguments.ipv4Address);
    addr.sin_port = htons(atoi(clientArguments.port));

    if(connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        //perror("connect");
        printErrorExit("Connection failed.\n");
    }

    sprintf(output,"Client-%d connecting to %s:%s\n",clientArguments.id,clientArguments.ipv4Address,clientArguments.port);
    printWithoutNewline(output);
}

void client_readQueryFile(){
    int i=0;
    char line[256];

    FILE *fp = fopen(clientArguments.pathToQueryFile,"r");
    if(fp == NULL){
        printErrorExit("Query File Error.\n");
    }

    while(fgets(line,sizeof(line),fp)){
        pthread_mutex_lock(&lock);

        client_sendQuery(line,&i);        

        pthread_mutex_unlock(&lock);
    }

    if(i==0 || i==1)
        sprintf(line,"A total of %d query were executed, client-%d is terminating.\n",i,clientArguments.id);
    else
        sprintf(line,"A total of %d queries were executed, client-%d is terminating.\n",i,clientArguments.id);
    printWithoutNewline(line);

    fclose(fp);
}

void client_sendQuery(char *line, int *count){
    int i=0,id=0;
    char *token,*rest=NULL;
    char output[250];
    Message message;

    for(token=strtok_r(line," ",&rest),i=0 ; token!=NULL ; token=strtok_r(NULL," ",&rest),i++){
        if(i == 0){
            if(isPositiveNumber(token) == -1){
                printErrorExit("Id number in the query file must be positive number.\n");
            }
            id = atoi(token);
            break;
        }
    }

    if(id == clientArguments.id){
        sprintf(output,"Client-%d connected and sending query '%s'\n",clientArguments.id,rest);
        printWithoutNewline(output);

        client_sendToServer(rest);

        recv(sfd, (Message *)&message, sizeof(message), 0);
        
        sprintf(output,"Server's response to Client-%d is %d records, and arrived in %.2f seconds.\n",clientArguments.id,message.records,message.seconds);
        printWithoutNewline(output);

        printWithoutNewline("\n");
        write(STDOUT_FILENO,message.buffer,strlen(message.buffer));
        fflush(stdout);
        printWithoutNewline("\n");

        (*count)++;

        //printf("\n%s\n",message.buffer);
    }
}

void client_sendToServer(char *message){
    send(sfd, message, strlen(message), 0);
}