#include "171044014_func_hw4.h"
#include "171044014_helper_hw4.h"
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pthread.h>
#include<semaphore.h>
#include<signal.h>

#define SIZE 150
#define MAX_STUDENT 50

char Queue[SIZE];
int front = 0;
int rear = -1;
int countQueue = 0;

int money;
int doneHomework = 0;

static sem_t countOffer;
static sem_t sem;
static sem_t finished;

sem_t semaphores[MAX_STUDENT];
pthread_t threads[MAX_STUDENT];
student students[MAX_STUDENT];

long fileSize;

char peek(){
    return Queue[front];
}

int offer(char data){
    if(countQueue == SIZE){
        printErrorExit("Queue Size Error: Queue has already MAX size\n");
    }
    rear = (rear == SIZE-1) ? -1 : rear;
    Queue[++rear] = data;
    ++countQueue;
    sem_post(&countOffer);
    return 1;
}

char poll(){
    char data = Queue[front++];
    front = (front == SIZE) ? 0 : front;
    --countQueue;
    return data;
}

int isEmpty(){
    return (countQueue == 0);
}

int isFinished(){
    return doneHomework == fileSize;
}

int isMoneyEnough(int size){
    int i=0;

    for(i=0;i<size;i++){
        if(money >= students[i].price){
            return 1;
        }
    }
    return 0;
}

void catchCtrlC(int sig_num){
    signal(SIGINT, catchCtrlC);
    printErrorExit("Exiting the program... (Because of CTRL+C interrupt)\n");
    fflush(stdout);
    fflush(stderr);
}

void initialErrorCheck(int argc,char **argv){
    if(argc != 4){
        printErrorExit("Invalid Number of Arguments: Number of arguments must have 4.\n");
    }

    if(isPositiveNumber(argv[3]) == -1){
        printErrorExit("Money Error: The third argument(Money) must be a positive number\n");
    }

    money = atoi(argv[3]);

    if(money <= 0){
        printErrorExit("Money Error: The third argument(Money) must be a positive number\n");
    }

    checkFileIsValid(argv[1]);
    checkFileIsValid(argv[2]);
}

void checkFileIsValid(const char *filename){
    struct stat st;
    char error[100];

    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        sprintf(error,"File Error: %s is not a valid file.\n",filename);
        printErrorExit(error);
    }
    if(stat(filename, &st) == 0){
        if(st.st_size == 0){
            sprintf(error,"File Size Error: %s can't be empty file.\n",filename);
            printErrorExit(error);
        }
    }

    //printf("%s: %ld\n",filename, st.st_size);

    close(fd);
}

void handleCommandLine(int argc,char **argv){
    int numberOfStudents,i=0,s=0,exitFlag;
    pthread_t thread1;
    struct stat st;

    stat(argv[1], &st);
    fileSize = st.st_size; 

    numberOfStudents = howManyLine(argv[2]);
    handleAllStudents(argv[2], students, numberOfStudents);
    semaphoresInits(numberOfStudents);
    createStudentThreads(numberOfStudents);

    s = pthread_create(&thread1, NULL, threadH, (void*) argv[1]);
    if(s != 0){
        printErrorExit("Pthread Create Error");
    }

    exitFlag = checkQueueValues(st.st_size,numberOfStudents);

    if(!isMoneyEnough(numberOfStudents)){
        exitTheFunc(2,numberOfStudents);
        return;
    }

    sem_wait(&finished);
    for(i=0;i<numberOfStudents;i++){
        pthread_cancel(threads[i]);
    }

    pthread_join(thread1,NULL);
    for(i=0;i<numberOfStudents;i++){
        pthread_join(threads[i],NULL);
    }

    exitTheFunc(exitFlag,numberOfStudents);
}

void exitTheFunc(int exitFlag, int numberOfStudents){
    char output[80];

    if(exitFlag == 1)
        sprintf(output,"No more homeworks left or coming in, closing.\n");
    else if(exitFlag == 2)
        sprintf(output,"Money is over, closing.\n");
    else
        sprintf(output,"Undefined exit flag value.\n");

    
    printWithoutNewline(output);
    printSolvedHomeworks(numberOfStudents);
    
    semaphoresDestroys(numberOfStudents);
}

int howManyLine(const char *filename){
    int res = 0;
    FILE *fp;
    char buffer[256];

    fp = fopen(filename, "r");

    while(fgets(buffer, sizeof(buffer), fp)){
        res++;
    }

    fclose(fp);
    return res;
}

void handleAllStudents(const char *filename, student students[] ,int numberOfStudents){
    int i=0;
    FILE *fp;
    char buffer[256];

    fp = fopen(filename, "r");

    while(fgets(buffer, sizeof(buffer), fp)){
        handleBuffer(buffer,students,numberOfStudents,i);
        ++i;
    }

    fclose(fp);
}

void createStudentThreads(int number){
    int i=0,s=0;

    for(i=0;i<number;i++){
        int *m = malloc(sizeof(int*));
        *m = i;
        s = pthread_create(&threads[i], NULL, threadStudent, (void *) m);
        if(s != 0){
            printErrorExit("Pthread Create Error");
        }
    }    
}

void selectStudent(char spec, student students[], int size){
    int selected=-1;

    // Critical Section
    selectNum(&selected,students,size,spec);
    sem_wait(&sem);
    if(selected != -1){
        if(!isMoneyEnough(size)){
            sem_post(&finished);
        }
        sem_post(&semaphores[selected]);
    } 
    sem_post(&sem);

}

void selectNum(int *selected, student students[], int size, char which){
    int i=0;

    sem_wait(&sem);
    for(i=0;i<size;i++){
        if(students[i].isAvailable == 1){
            if(*selected == -1 && money >= students[i].price){
                *selected = i;
            } 

            else if(*selected != -1){
                switch(which){
                    case 'Q':
                        if(students[*selected].quality < students[i].quality && money >= students[i].price){
                            *selected = i;
                        }
                        break;
                    case 'S':
                        if(students[*selected].speed < students[i].speed && money >= students[i].price){
                            *selected = i;
                        }
                        break;
                    case 'C':
                        if(students[*selected].price > students[i].price && money >= students[i].price){
                            *selected = i;
                        }
                        break;
                }
            }
        }
    }
    sem_post(&sem);
}

int isAllUnavailable(int size){
    int i=0;
    for(i=0;i<size;i++){
        if(students[i].isAvailable == 1){
            return 0;
        }
    }
    return 1;
}

void handleBuffer(char *buf, student students[], int size, int num){
    char *tok;
    int i=0;
    while ((tok = strtok_r(buf, " ", &buf))){
        switch(i){
            case 0:
                sprintf(students[num].name,"%s",tok);
                break; //
            case 1:
                if(!isPositiveNumber(tok)){
                    printErrorExit("Quality Number Error: Quality Number must be a positive number");
                }
                students[num].quality = atoi(tok);
                break;
            case 2:
                if(!isPositiveNumber(tok)){
                    printErrorExit("Speed Number Error: Speed Number must be a positive number");
                }
                students[num].speed = atoi(tok);
                break;
            case 3:
                if(!isPositiveNumber(tok)){
                    printErrorExit("Price Number Error: Price Number must be a positive number");
                }
                students[num].price = atoi(tok);
                break;
        }
        ++i;
    }
    students[num].isAvailable = 1;
    students[num].done = 0;

    //printf("X : %s %d %d %d\n",name,quality,speed,price);
}

int checkQueueValues(long size, int numberOfStudents){
    int exitFlag1=0,exitFlag2=0;
    char ch;

    while(1){
        sem_wait(&countOffer);

        sem_wait(&sem);
        if(!isMoneyEnough(numberOfStudents)){
            exitFlag2=1;
            sem_post(&sem);
            break;
        }
        sem_post(&sem);

        ch = poll();

        if(ch == 'N'){
            exitFlag1=1;
            break;
        }

        selectStudent(ch,students,numberOfStudents);
    }

    if(exitFlag1 == 1){
        return 1;
    }

    if(exitFlag2 == 1){
        return 2;
    }

    return 0;
}

void semaphoresInits(int numberOfStudents){
    int i=0;

    for(i=0;i<numberOfStudents;++i){
        if(sem_init(&semaphores[i], 0, 0) == -1){
            printErrorExit("Error sem_init...\n");
        }
    }
    if(sem_init(&sem, 0, 1) == -1){
        printErrorExit("Error sem_init 2");
    }
    if(sem_init(&countOffer, 0, 0) == -1){
        printErrorExit("Error sem_init 3");
    }
    if(sem_init(&finished, 0, 0) == -1){
        printErrorExit("Error sem_init 3");
    }
}

void semaphoresDestroys(int numberOfStudents){
    int i=0;

    for(i=0;i<numberOfStudents;++i){
        sem_destroy(&semaphores[i]);
    }
    sem_destroy(&sem);
    sem_destroy(&countOffer);
    sem_destroy(&finished);
}

void printSolvedHomeworks(int size){
    char output[128];

    sem_wait(&sem);
    sprintf(output,"Homeworks solved and money made by the students:\n");
    printWithoutNewline(output);
    for(int i=0;i<size;++i){
        sprintf(output,"%s %d %d\n",students[i].name, students[i].done, students[i].done*students[i].price);
        printWithoutNewline(output);
    }
    sprintf(output,"Money left at H's account: %d TL\n",money);
    printWithoutNewline(output);
    sem_post(&sem);
}

void *threadStudent(void *arg){
    char output[128];
    int num,sleepTime;

    num = *(int*) arg;
    sleepTime = 6 - students[num].speed;

    while(!isFinished()){
        sprintf(output,"%s is waiting for a homework\n",students[num].name);
        printWithoutNewline(output);

        sem_wait(&semaphores[num]);

        sem_wait(&sem);
        if(isFinished()){
            sem_post(&finished);
            sem_post(&sem);
            return NULL;
        }

        if(money < students[num].price){
            sem_post(&sem);
            break;
        }

        money -= students[num].price;
        sprintf(output,"%s is solving homework X for %d, H has %d TL left.\n",students[num].name,students[num].price,money);
        printWithoutNewline(output);
        doneHomework += 1;
        students[num].done += 1;
        students[num].isAvailable = 0;
        sem_post(&sem);

        sleep(sleepTime);

        sem_wait(&sem);
        if(money < students[num].price){
            sem_post(&sem);
            break;
        }

        students[num].isAvailable = 1;
        sem_post(&sem);

        sem_post(&semaphores[num]);
    }
    
    sem_post(&finished);
    return NULL;
}

void *threadH(void *arg){
    char output[120];
    char *filename;
    char c;
    int fd;

    filename = (char*) arg;
    fd = open(filename, O_RDONLY);

    while(read(fd,&c,1) == 1){
        sem_wait(&sem);

        if(money == 0){
            printWithoutNewline("H has no more money for homeworks, terminating.\n");
            sem_post(&sem);
            return NULL;
        }
        sprintf(output,"H has a new homework %c; remaining money is %d\n",c,money);
        printWithoutNewline(output);
        offer(c);

        sem_post(&sem);
    }
    offer('N');

    close(fd);

    printWithoutNewline("H has no other homeworks terminating.\n");
    return NULL;
}

