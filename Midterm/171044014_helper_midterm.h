#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<string.h>

struct Buffer{
    int _1Vaccine;
    int _2Vaccine;
};

void printErrorExit(const char *errMessage){
    write(STDERR_FILENO,errMessage,strlen(errMessage));
    _exit(1);
}

void printWithNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
    write(STDOUT_FILENO,"\n",1);
    fflush(stdout);
}

void printWithoutNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
    fflush(stdout);
}

int isPositiveNumber(const char *str){
    while(*str){
        if(*str >= '0' && *str <= '9'){
            str++;
        } else {
            return -1;   
        }
    }
    return 1;
}

void openFD(int *fd, const char *name, int size){
    struct stat bb;
    *fd = shm_open(name, (O_RDWR | O_CREAT) , 00600);
    if(*fd == -1){
        printErrorExit("Error: File descriptor in line 514\n");
    }

    fstat(*fd,&bb);

    if(bb.st_size != size){
        if(ftruncate(*fd,size) == -1){
            printErrorExit("Error: Ftruncate in line 521\n");
        }
    }
}

void getVaccineFromShared(const char *sharedName, int size, int *vaccine1, int *vaccine2){
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

    *vaccine1 = buffer[0]._1Vaccine;
    *vaccine2 = buffer[0]._2Vaccine;
}



