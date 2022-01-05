#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<semaphore.h>
#include"171044014_helper_midterm.h"

struct Request{
    int num;
};

int main(int argc, char **argv){
    char output[250];
    int fd=0,dummyFd=0,which=0,size=0,tsize=0,i=0,vaccine1=0,vaccine2=0;
    struct Request request;
    char fifoPath[20];
    which = atoi(argv[0]);
    tsize = atoi(argv[1]);
    size = atoi(argv[2]);
    sem_t sem;

    sprintf(fifoPath,"/tmp/tmp%d",which);

    sem_wait(&sem);

    umask(0);
    if(mkfifo(fifoPath,S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        printErrorExit("Error: Mkfifo in line 254");
    }

    fd = open(fifoPath, O_RDWR);
    if(fd == -1){
        printErrorExit("Error: Open in line 259");
    }

    dummyFd = open(fifoPath, O_RDWR);
    if (dummyFd == -1){
        printErrorExit("Error: dummy fd open in line 259");
    }

    sem_post(&sem);

    for(i=0;i<tsize;++i){
        if(read(fd,&request,sizeof(struct Request)) != sizeof(struct Request)){
            fprintf(stderr,"Error reading request, discarding\n");
            continue;
        }

        getVaccineFromShared("Buffer",size,&vaccine1,&vaccine2);

        if(i == tsize - 1){
            sprintf(output,"Citizen %d (pid=%d) is vaccinated for the %d.time: the clinic has %d vaccine1 and %d vaccine2. The citizen is leaving.\n",(which+1),getpid(),(i+1),vaccine1,vaccine2);
            printWithoutNewline(output);
        } else{
            sprintf(output,"Citizen %d (pid=%d) is vaccinated for the %d.time: the clinic has %d vaccine1 and %d vaccine2.\n",(which+1),getpid(),(i+1),vaccine1,vaccine2);
            printWithoutNewline(output);
        }
        continue;

    }

    close(fd);

    //printf("Citizen\n");

    return 0;
}
