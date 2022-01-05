#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<semaphore.h>
#include"171044014_helper_midterm.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

void readFileChByCh(int fd, sem_t *m_sem, sem_t *semaphore, sem_t *full1, sem_t *full2, sem_t *empty1, sem_t *empty2, int nurse, int size);
void writeSharedMem(const char *sharedName, int size, int which);
int getAndIncrementSem(sem_t *semaphore);
int getHowManyWacInFile(const char *filename);

int main(int argc, char **argv){
    char output[75];
    int size = 0, csize=0, tsize=0, totalVac = 0, nowVac = 0;
    sem_t *semaphore,*full1,*full2,*empty1,*empty2,*m_sem;

    csize = atoi(argv[5]);
    tsize = atoi(argv[6]);
    size = csize * tsize;
    
    semaphore = sem_open(argv[3],(O_CREAT | O_RDWR),SEM_PERMS,0);
    m_sem = sem_open(argv[4],(O_CREAT | O_RDWR),SEM_PERMS,1);
    full1 = sem_open("full1",(O_CREAT | O_RDWR),SEM_PERMS,0);
    full2 = sem_open("full2",(O_CREAT | O_RDWR),SEM_PERMS,0);
    empty1 = sem_open("empty1",(O_CREAT | O_RDWR),SEM_PERMS,size);
    empty2 = sem_open("empty2",(O_CREAT | O_RDWR),SEM_PERMS,size);
    
    totalVac = getHowManyWacInFile(argv[2]);
    sem_getvalue(semaphore,&nowVac);

    //printf("-\n",nowVac,totalVac);

    int fd = open(argv[2], O_RDONLY);
    if(fd == -1){
        printErrorExit("Hata");
    }

    while(nowVac != totalVac){
        readFileChByCh(fd, m_sem, semaphore, full1, full2, empty1, empty2, atoi(argv[1]),size);
        sem_getvalue(semaphore,&nowVac);
    }

    //writeSharedMem(size,1);
    sprintf(output,"The nurse is terminating.\n");
    printWithoutNewline(output);

    sem_close(semaphore);
    sem_close(full1);
    sem_close(full2);
    sem_close(empty1);
    sem_close(empty2);
    
    close(fd);

    return 0;
}

void readFileChByCh(int fd, sem_t *m_sem ,sem_t *semaphore, sem_t *full1, sem_t *full2, sem_t *empty1, sem_t *empty2, int nurse, int size){
    char output[100];
    char c;
    int getNum=0, i=0, vaccine1=0, vaccine2=0;
    struct flock lock;

    memset(&lock,0,sizeof(lock));
    lock.l_type = F_RDLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        perror("p");
        printErrorExit("Error: Filelock is not set\n");
    }

    getNum = getAndIncrementSem(semaphore);

    
    /* TO_DO */
    lseek(fd,0,SEEK_SET);
    while(read(fd,&c,1) == 1){
        if(i == getNum){
            //printf("%c,",c);

            if(c == '1')
                sem_wait(empty1);

            else if(c == '2')
                sem_wait(empty2);
            
            sem_wait(m_sem);
            writeSharedMem("Buffer",size,c - '0');
            sem_post(m_sem);

            if(c == '1')
                sem_post(full1);

            else if(c == '2')
                sem_post(full2);

            getVaccineFromShared("Buffer",size,&vaccine1,&vaccine2);
            
            sprintf(output,"Nurse %d (pid=%d) has brought vaccine %c: the clinic has %d vaccine1 and %d vaccine2\n",nurse+1,getpid(),c,vaccine1,vaccine2);
            printWithoutNewline(output);
            break;
        }
        i++;
    }


    lock.l_type = F_UNLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        printErrorExit("Error: Filelock is not unlocked\n");
    }
}

void writeSharedMem(const char *sharedName, int size, int which){
    void *addr;
    int fd;
    struct Buffer *buffer;

    openFD(&fd,sharedName,size*2);

    addr = mmap(0,size*2,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 124\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 127\n");
    }

    buffer = (struct Buffer*)addr; 

    if(which == 1)
        buffer[0]._1Vaccine += 1;   
    else if(which == 2)
        buffer[0]._2Vaccine += 1;
}

int getAndIncrementSem(sem_t *semaphore){
    int res = 0;

    sem_getvalue(semaphore,&res);
    sem_post(semaphore);

    return res;
}

int getHowManyWacInFile(const char *filename){
    char c;
    int i=0;
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        printErrorExit("File Description Error\n");
    }

    while(read(fd,&c,1) == 1){
        if(c == '1' || c == '2')
            i++;
    }

    return i;
}