#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<signal.h>
#include<time.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<semaphore.h>
#include<errno.h>
#include"171044014_helper_hw3.h"

#define true 1
#define false 0
#define fail -1
#define finish -333
#define SHRD_MEM_TOTAL_SIZE 4096
#define SHRD_MEM_TEMPLATE "%ld : %d\n"
#define SHRD_MEM_LEN (sizeof(SHRD_MEM_TEMPLATE) + 20)
#define MAX_FIFO 20

struct Process{
    pid_t pid;
    int numberOfSwitches;
    char fifoName[20];
    int totalSwitch;
};

struct Request{
    pid_t pid;
    char fifoName[20];
};

void handleCommandLine(int argc, char **argv);
void handleTheArgs(const char *bargs, const char *sargs, const char *fargs, const char *margs);
pid_t writeToSharedMem(sem_t *semaphore, const char *sargs, int numberOfSwitches, char *allFifoNames[], int length, char *fifoPath);
int randomNonusedFifo(struct Process *processes,char *allFifoNames[], int length);
void openFifoWait(const char *sargs, char *fifoPath, pid_t pid, int length, sem_t *semaphore);
int sendPotato(struct Request request, const char *sargs, int length);
pid_t chooseRandomFifo(struct Request request, const char *sargs, int length);
bool arePotatoesAll(const char *sargs);
bool arePotatoesFinished(const char *sargs);
bool isThePotatoFinished(struct Request request, const char *sargs);
int decrementPotato(const char *sargs, struct Request request);
void getFifoPath(const char *sargs, pid_t pid, char *fifoPath);
void openX(int *fd, const char *sargs);
int getTotalSwitchAndIncrement(const char *sargs, pid_t pid);
void swapProcesses(const char *sargs, char *fifoName, pid_t requestPid);
void finishSignal(const char *sargs, pid_t pid);

void catchCtrlC(int sig_num){
    printErrorExit("Exiting the program.. (Because of CTRL+C interrupt)\n");
}

int main(int argc, char **argv){
    signal(SIGINT, catchCtrlC);

    handleCommandLine(argc,argv);

    return 0;
}

void handleCommandLine(int argc, char **argv){
    int opt=0;
    int bflag=0, sflag=0, fflag=0, mflag=0;
    const char *bargs=NULL, *sargs=NULL, *fargs=NULL, *margs=NULL;

    while((opt = getopt(argc, argv, "b:s:f:m:")) != -1) {  
        switch(opt) { 
            case 'b': 
                if(isPositiveNumber(optarg) == -1){
                    printErrorExit("Wrong parameter argument: -b flag's argument has to be positive number or zero.\n");
                }
                bargs = optarg;
                bflag++;
                break;
            case 's': 
                sargs = optarg;
                sflag++;
                break;
            case 'f':
                fargs = optarg;
                fflag++;
                break;
            case 'm':
                margs = optarg;
                mflag++;
                break;
        }
    }

    if(bflag > 1 || sflag > 1 || fflag > 1 || mflag > 1){
        printErrorExit("Each flag can only use one time.(Flags : -b, -s, -f, -m) \n");
    }

    if(bflag == 0 || sflag == 0 || fflag == 0 || mflag == 0){
        printErrorExit("Each flag can only use one time.(Flags : -b, -s, -f, -m) \n");
    }
    handleTheArgs(bargs,sargs,fargs,margs);
}

void handleTheArgs(const char *bargs, const char *sargs, const char *fargs, const char *margs){
    int numberOfSwitches = atoi(bargs);
    char buffer[100];
    char *allFifoNames[MAX_FIFO];
    char *line;
    char fifoPath[20];
    int i=0,j=0,nn=0;
    pid_t pid;
    sem_t *semaphore;
    struct Request request;

    semaphore = sem_open(margs,(O_CREAT | O_RDWR),0644,0);

    FILE *fp = fopen(fargs,"a+"); 

    // Gets all fifonames in fileWithFifoNames
    while(fgets(buffer, sizeof(buffer), fp)){
        line = (char *)malloc(sizeof(buffer));
        strcpy(line,buffer);
        if(line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = '\0';
        allFifoNames[i] = line;
        i++;
    }

    pid = writeToSharedMem(semaphore,sargs,numberOfSwitches,allFifoNames,i,fifoPath);
    if(pid == -1){
        printf("All processes are already created, Not allowed to create a new process\n");
    }

    sem_getvalue(semaphore,&nn);
    while(nn != i){
        sem_getvalue(semaphore,&nn);
    }

    request.pid = pid;
    memcpy(request.fifoName,fifoPath,strlen(fifoPath));

    if(arePotatoesFinished(sargs)){
        printErrorExit("Argument Error: You must start at least one process with a potato.\n");
    }

    if(arePotatoesAll(sargs)){
        printErrorExit("Argument Error: You must start at least one process with empty hands.\n");
    }

    if(numberOfSwitches == 0){
        sem_wait(semaphore);
        openFifoWait(sargs, fifoPath, pid, i, semaphore);
        sem_post(semaphore);

    } else{
        sem_wait(semaphore);
        sendPotato(request, sargs, i);
        openFifoWait(sargs, fifoPath, pid, i, semaphore);
        sem_post(semaphore);
    }

    // Free all names of the fifos spaces
    for(j=0;j<i;++j){
        free(allFifoNames[j]);
    }

    // Close fp and semaphore
    fclose(fp);
    sem_unlink(margs);
    sem_close(semaphore);
    shm_unlink(sargs);

    exit(EXIT_SUCCESS);
}

pid_t writeToSharedMem(sem_t *semaphore,const char *sargs, int numberOfSwitches, char *allFifoNames[], int length, char *fifoPath){
    int i=0,flag=1;

    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 183\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 186");
    }
    processes = (struct Process*)addr;

    // Create a struct in order to write to shared memory
    struct Process theProcess;
    theProcess.pid = getpid();
    theProcess.numberOfSwitches = numberOfSwitches;
    theProcess.totalSwitch = 0;

    
    int rnd = randomNonusedFifo(processes,allFifoNames,length);
    if(rnd == -1){
        flag=0;
        //printErrorExit("Fifo nums ran out\n");
    }

    i=0;
    while(processes[i].pid != 0){
        //printf("already added: %d - %d - %s\n",processes[i].pid, processes[i].numberOfSwitches, processes[i].fifoName);
        ++i;
    }

    // Add to shared memory the string of the potato.
    if(flag==1){

        int _strlen = strlen(allFifoNames[rnd]);
        memcpy(theProcess.fifoName, allFifoNames[rnd], _strlen);
        theProcess.fifoName[_strlen] = '\0';
        processes[i] = theProcess;
        //printf("%d . added: %d - %d - %s\n",i,theProcess.pid,theProcess.numberOfSwitches,theProcess.fifoName);
        memcpy(fifoPath, theProcess.fifoName, _strlen);
        fifoPath[_strlen] = '\0';

        sem_post(semaphore);

        return theProcess.pid;
    }

    return -1;
}

int randomNonusedFifo(struct Process *processes,char *allFifoNames[], int length){
    int randomNum=0,i=0,flag=1;
    srand(time(NULL));

    do{
        randomNum = rand() % length;

        i=0;
        flag=1;
        while(processes[i].pid != 0){
            if(strcmp(processes[i].fifoName,allFifoNames[randomNum]) == 0){
                flag=0;
                break;
            }
            ++i;
        }

        if(flag == 1){
            return randomNum;
        } 
        if(i == length){
            return -1;
        }
    } while (1);
    
    return -1;
}

void openFifoWait(const char *sargs, char *fifoPath, pid_t pid, int length, sem_t *semaphore){
    int fd=0,i=0,_strlen=0,dec=0;
    struct Request request;

    umask(0);
    if(mkfifo(fifoPath,S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        printErrorExit("Error: Mkfifo in line 254");
    }

    fd = open(fifoPath, O_RDWR);
    if(fd == -1){
        printErrorExit("Error: Open in line 259");
    }

    for(;;){
        if(arePotatoesFinished(sargs)){
            finishSignal(sargs,pid);
            break;
        }

        if(read(fd,&request,sizeof(struct Request)) != sizeof(struct Request)){
            fprintf(stderr,"Error reading request, discarding\n");
            continue;
        } 

        if(request.pid != finish){
            printf("pid = %d receiving potato number %d from %s\n",pid,request.pid,request.fifoName);

            dec = decrementPotato(sargs, request);
            if(dec == -1){
                continue;
            } 

            _strlen = strlen(fifoPath);
            swapProcesses(sargs,fifoPath,request.pid);
            memcpy(request.fifoName,fifoPath,_strlen);
            request.fifoName[_strlen] = '\0';

            int sending = sendPotato(request,sargs,length);
            ++i;
            if(sending == fail){
                continue;
            }
        }
    }
}

int sendPotato(struct Request request, const char *sargs, int length){
    int totalSwitch=0;
    pid_t randomlyPid;
    char fifoPath[20];

    randomlyPid = chooseRandomFifo(request,sargs,length);
    totalSwitch = getTotalSwitchAndIncrement(sargs,randomlyPid);
    getFifoPath(sargs,randomlyPid,fifoPath);
    if(randomlyPid == -1){
        return -1;
    }

    // Open fifo
    int fd_fifo = open(fifoPath, O_RDWR);
    if(fd_fifo == -1){
        return -1;
    }
    if(write(fd_fifo,&request,sizeof(struct Request)) != sizeof(struct Request)){
        fprintf(stderr,"Error writing to FIFO %s\n",fifoPath);
        return -1;
    } else{
        printf("pid = %d sending potato number %d to %s; this is swith num %d\n",getpid(),request.pid,fifoPath,totalSwitch);
    }

    if(close(fd_fifo) == -1){
        return -1;
    }

    return 0;
}

pid_t chooseRandomFifo(struct Request request, const char *sargs, int length){
    struct Process *processes;
    int i=0,randomNum;
    void *addr;
    int fd;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 336\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 339\n");
    }
    processes = (struct Process*)addr;

    srand(time(NULL));

    do{
        if(i > 1000){
            printf("Any processes is not empty\n");
            return -1;
        }
        randomNum = rand() % length;

        if(request.pid != processes[randomNum].pid){
            return processes[randomNum].pid;
        }
        ++i;
    }while(request.pid == processes[randomNum].pid);

    return -1;
}

bool arePotatoesAll(const char *sargs){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 371\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 374\n");
    }
    processes = (struct Process*)addr;
    
    while(processes[i].pid != 0){
        if(processes[i].numberOfSwitches == 0){
            return false;
        }
        ++i;
    }

    return true;
}

bool arePotatoesFinished(const char *sargs){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 398\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 401\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].numberOfSwitches > 0){
            return false;
        }
        ++i;
    }

    return true;
}

bool isThePotatoFinished(struct Request request, const char *sargs){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 425\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 428\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].pid == request.pid){
            if(processes[i].numberOfSwitches <= 0){
                return true;
            }
        }
        ++i;
    }

    return false;
}

int decrementPotato(const char *sargs, struct Request request){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 454\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 457\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].pid == request.pid){
            if(processes[i].numberOfSwitches <= 0){
                printf("Error: Number of swiches can't be less than zero\n");
                return -1;
            } else{
                processes[i].numberOfSwitches -= 1;
                if(isThePotatoFinished(request,sargs)){
                    printf("pid = %d; potato number %d has cooled down\n",getpid(),request.pid);
                    return -1;
                }
                return 1;
            }
        }
        ++i;
    }

    return -1;
}

void getFifoPath(const char *sargs, pid_t pid, char *fifoPath){
    int i=0,_strlen=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 491\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 494\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].pid == pid){
            _strlen = strlen(processes[i].fifoName);
            memcpy(fifoPath, processes[i].fifoName, _strlen);
            fifoPath[_strlen] = '\0';
            return;
        }

        ++i;
    }
}

void openX(int *fd, const char *sargs){
    struct stat bb;
    *fd = shm_open(sargs, (O_RDWR | O_CREAT) , 00600);
    if(*fd == -1){
        printErrorExit("Error: File descriptor in line 514\n");
    }

    fstat(*fd,&bb);

    if(bb.st_size != SHRD_MEM_TOTAL_SIZE){
        if(ftruncate(*fd,SHRD_MEM_TOTAL_SIZE) == -1){
            printErrorExit("Error: Ftruncate in line 521\n");
        }
    }
}

int getTotalSwitchAndIncrement(const char *sargs, pid_t pid){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 536\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Mmap in line 539\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].pid == pid){
            return ++processes[i].totalSwitch;
        }

        ++i;
    }

    return -1;
}

void swapProcesses(const char *sargs, char *fifoName, pid_t requestPid){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;
    struct Process newp;
    struct Process newp2;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 566\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 569\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(strcmp(processes[i].fifoName,fifoName) == 0){
            newp.numberOfSwitches = processes[i].numberOfSwitches;
            newp.pid = processes[i].pid;
            newp.totalSwitch = processes[i].totalSwitch;
        }
        ++i;
    }
    i=0;
    while(processes[i].pid != 0){
        if(processes[i].pid == requestPid){
            newp2.numberOfSwitches = processes[i].numberOfSwitches;
            newp2.pid = processes[i].pid;
            newp2.totalSwitch = processes[i].totalSwitch;

            processes[i].numberOfSwitches = newp.numberOfSwitches;
            processes[i].pid = newp.pid;
            processes[i].totalSwitch = newp.totalSwitch;
        }
        ++i;
    }
    i=0;
    while(processes[i].pid != 0){
        if(strcmp(processes[i].fifoName,fifoName) == 0){
            processes[i].numberOfSwitches = newp2.numberOfSwitches;
            processes[i].pid = newp2.pid;
            processes[i].totalSwitch = newp2.totalSwitch;
        }
        ++i;
    }

}

void finishSignal(const char *sargs, pid_t pid){
    int i=0;
    void *addr;
    int fd;
    struct Process *processes;
    struct Request request;
    request.pid = finish;

    openX(&fd,sargs);

    addr = mmap(0,SHRD_MEM_TOTAL_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 618\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Mmap in line 621\n");
    }
    processes = (struct Process*)addr;

    while(processes[i].pid != 0){
        if(processes[i].pid != pid){
            int fd_fifo = open(processes[i].fifoName, O_RDWR);
            if(fd_fifo == -1){
                continue;
            }
            if(write(fd_fifo,&request,sizeof(struct Request)) != sizeof(struct Request)){
                fprintf(stderr,"Error writing to FIFO %s\n",processes[i].fifoName);
            } 
        }
        ++i;
    }
}