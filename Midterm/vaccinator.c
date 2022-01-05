#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<errno.h>
#include"171044014_helper_midterm.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

struct Request{
    int num;
};

struct Citizen{
    pid_t pid;
    int remaining;
};

void citizenCall(const char *sharedName, int size, int carg, int which, int which2);
int sendToFifo(int index, pid_t pid, int which);
void vaccineShot(const char *sharedName, int size, int which);

int main(int argc, char **argv){
    int which=0,size=0,csize=0,tsize=0,i=0,value=0;
    sem_t *semaphore,*empty1,*full1,*empty2,*full2,*m_sem;
    //char fifoPath[20];

    size = atoi(argv[0]);
    csize = atoi(argv[1]);
    tsize = atoi(argv[2]);
    which = atoi(argv[5]);

    semaphore = sem_open(argv[3],(O_CREAT | O_RDWR),SEM_PERMS,0);
    m_sem = sem_open(argv[4],(O_CREAT | O_RDWR),SEM_PERMS,1);
    full1 = sem_open("full1",(O_CREAT | O_RDWR),SEM_PERMS,0);
    full2 = sem_open("full2",(O_CREAT | O_RDWR),SEM_PERMS,0);
    empty1 = sem_open("empty1",(O_CREAT | O_RDWR),SEM_PERMS,tsize*csize);
    empty2 = sem_open("empty2",(O_CREAT | O_RDWR),SEM_PERMS,tsize*csize);

    sem_getvalue(semaphore,&value);
    while(value < tsize*csize - 1){
        if(errno == ENOENT){
            printErrorExit("");
        }
        sem_wait(full1);
        sem_wait(m_sem);
        // Citizen Call
        citizenCall("Citizens",size,csize,which,1);
        sem_post(semaphore);
        sem_post(m_sem);
        sem_post(empty1);

        sem_wait(full2);
        sem_wait(m_sem);
        citizenCall("Citizens",size,csize,which,2);
        sem_post(m_sem);
        sem_post(empty2);

        sem_getvalue(semaphore,&value);
        i++;
    }

    //printf("Vaccinator Finished!!!! \n");

    sem_close(full1);
    sem_close(full2);
    sem_close(empty1);
    sem_close(empty2);
    sem_close(m_sem);

    return 0;
}

void citizenCall(const char *sharedName, int size, int carg, int which, int which2){
    void *addr;
    int fd,i=0;
    struct Citizen *citizens;
    int ctsize = carg * (sizeof(struct Citizen));

    openFD(&fd,sharedName,ctsize);

    addr = mmap(0,ctsize,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 183\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 186");
    }

    citizens = (struct Citizen*)addr; 

    if(which2 == 1){

        while(citizens[i].pid != 0){
            if(citizens[i].remaining > 0){
                if( sendToFifo(i,citizens[i].pid,which) == -1 ){
                    perror("p");
                    printErrorExit("Hata send to fifo\n");
                } else{
                    citizens[i].remaining -= 1;
                    vaccineShot("Buffer",size,which2);
                    break;
                }
            }
            //printf("Citizen[%d] : %d - %d",i,citizens[i].pid,citizens[i].remaining);
            //printf("already added: %d - %d - %s\n",processes[i].pid, processes[i].numberOfSwitches, processes[i].fifoName);
            ++i;
        }

    } else if(which2 == 2){
        vaccineShot("Buffer",size,which2);
    }
}

int sendToFifo(int index, pid_t pid, int which){
    char output[100];
    struct Request req;
    char fifoPath[25];
    int fd_fifo=0;

    sprintf(fifoPath,"/tmp/tmp%d",index);

    fd_fifo = open(fifoPath, O_WRONLY);
    while(fd_fifo == -1){
        fd_fifo = open(fifoPath, O_WRONLY);
    }
    /*if(fd_fifo == -1){
        return -1;
    }*/
    req.num = index;

    if(write(fd_fifo,&req,sizeof(struct Request)) != sizeof(struct Request)){
        fprintf(stderr,"Error writing to FIFO %s\n",fifoPath);
        return -1;
    } else{
        sprintf(output,"Vaccinator %d (pid=%d) is inviting citizen pid=%d to the clinic\n",(which+1),getpid(),pid);
        printWithoutNewline(output);
    }

    if(close(fd_fifo) == -1){
        return -1;
    }

    return 0;
}

void vaccineShot(const char *sharedName, int size, int which){
    void *addr;
    int fd;
    struct Buffer *buffer;

    openFD(&fd,sharedName,size*2);

    addr = mmap(0,size*2,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 183\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 186");
    }

    buffer = (struct Buffer*)addr; 

    if(which == 1){
        if(buffer[0]._1Vaccine <= 0){
            printErrorExit("Error: No vaccine 1 in shared memory\n");
        }
        buffer[0]._1Vaccine -= 1;
    } else if(which == 2){
        if(buffer[0]._2Vaccine <= 0){
            printErrorExit("Error: No vaccine 2 in shared memory\n");
        }
        buffer[0]._2Vaccine -= 1;

    }
}