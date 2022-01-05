#include "171044014_func_midterm.h"
#include "171044014_helper_midterm.h"
#include<semaphore.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<unistd.h>
#include<fcntl.h>

int co=0;
pid_t vaccinators[100];

struct Citizen{
    pid_t pid;
    int remaining;
};

void catchChild(int sig_num){
    int status;
    wait(&status);
}

void catchCtrlC(int sig_num){
    printErrorExit("Exiting the program.. (Because of CTRL+C interrupt)\n");
}

void whetherPositiveNumber(const char *arg, char val){
    char error[100];

    sprintf(error, "Wrong parameter argument: -%c flag's argument has to be positive number or zero.\n", val);
    if(isPositiveNumber(arg) == -1){
        printErrorExit(error);
    }
}

void whetherGreaterThan(int num1, int num2, char val){
    char error[100];

    sprintf(error, "Wrong parameter argument: -%c flag's argument has to be greater than or equal to %d.\n", val, num2);
    if(num1 < num2){
        printErrorExit(error);
    }
}

void initialErrorCheck(int argc,char **argv){
    if(argc < 2 || argc > 13){
        printErrorExit("Wrong number of parameter: Each flag can only use one time.(Flags : -n, -v, -c, -b, -t, -i)\n");
    }
}

void handleCommandLine(int argc, char **argv){
    int opt=0, totalProcess=0;
    int nflag=0, vflag=0, cflag=0, bflag=0, tflag=0, iflag=0;
    int narg,varg,carg,barg,targ;
    char *iarg=NULL;
    char *semaphoreNames[] = {"CountVac10","empty1","empty2","full1","full2","CountVac11","MSem"};
    char output[120];

    while((opt = getopt(argc, argv, "n:v:c:b:t:i:")) != -1) {  
        switch(opt) { 
            case 'n': 
                whetherPositiveNumber(optarg,'n');
                narg = atoi(optarg);
                whetherGreaterThan(narg,2,'n');
                nflag++;
                break;
            case 'v': 
                whetherPositiveNumber(optarg,'v');
                varg = atoi(optarg);
                whetherGreaterThan(varg,2,'v');
                vflag++;
                break;
            case 'c':
                whetherPositiveNumber(optarg,'c');
                carg = atoi(optarg);
                whetherGreaterThan(carg,3,'c');
                cflag++;
                break;
            case 'b':
                whetherPositiveNumber(optarg,'b');
                barg = atoi(optarg);
                bflag++;
                break;
            case 't':
                whetherPositiveNumber(optarg,'t');
                targ = atoi(optarg);
                whetherGreaterThan(carg,1,'c');
                tflag++;
                break;
            case 'i':
                iarg = optarg;
                iflag++;
                break;
        }
    }

    if(nflag > 1 || vflag > 1 || cflag > 1 || bflag > 1 || tflag > 1 || iflag > 1){
        printErrorExit("Each flag can only use one time.(Flags : -n, -v, -c, -b, -t, -i) \n");
    }

    if(nflag == 0 || vflag == 0 || cflag == 0 || bflag == 0 || tflag == 0 || iflag == 0){
        printErrorExit("Each flag must use one time.(Flags : -n, -v, -c, -b, -t, -i) \n");
    }
    //handleTheArgs(bargs,sargs,fargs,margs);

    if(barg < targ*carg+1){
        printErrorExit("Buffer size must be larger than or equal to t*c+1\n");
    }

    int fd = open(iarg, O_RDONLY);
    if(fd == -1){
        printErrorExit("File Error: The file is not correct file\n");
    }
    close(fd);

    unlinkResources(semaphoreNames,7);

    sprintf(output,"Welcome to the GTU344 clinic. Number of citizens to vaccinate c=%d with t=%d doses.\n",carg,targ);
    printWithoutNewline(output);

    createCitizen(carg,targ,barg);
    createNurse(narg,barg,targ,carg,iarg,semaphoreNames[0],semaphoreNames[6]);
    createVaccinator(varg,barg,carg,targ,iarg,semaphoreNames[5],semaphoreNames[6]);

    totalProcess = carg + varg + narg;
    waitAllProcesses(totalProcess);

    if(allCitizensFinished("Citizens",carg)){
       killVaccinators();
    }

    unlinkResources(semaphoreNames,7);
}

void waitAllProcesses(int totalProcess){
    while(totalProcess){
        waitpid(WAIT_ANY,NULL,0);
        totalProcess--;
    }
}

void unlinkResources(char *semaphoreNames[], int size){
    int i=0;

    shm_unlink("Buffer");
    shm_unlink("Citizens");
    for(;i<size;++i){
        sem_unlink(semaphoreNames[i]);
    }
}

int createAProcess(char *name, char *arg[]){
    pid_t childPid;

    childPid = fork();
    if(childPid != 0){
        //waitpid(-1,NULL,0);
        if(strcmp(name,"vaccinator") == 0)
            vaccinators[co++] = childPid;
        return childPid;
    } else{
        execv(name,arg);
        printErrorExit("Creating Process Error: The process is not created(execv)\n");
    }

    return -1;
}

void createNurse(int narg, int barg, int targ, int carg, char *filename, char *sem1, char *sem2){
    int i=0;
    char size[5],count[5],csize[5],tsize[5];

    sprintf(size,"%d",barg);
    sprintf(csize,"%d",carg);
    sprintf(tsize,"%d",targ);

    for(;i<narg;++i){
        sprintf(count,"%d",i); 
        createAProcess("nurse", (char *[]){size,count,filename,sem1,sem2,tsize,csize,NULL});
    }
}

void createVaccinator(int varg, int barg, int carg, int targ, char *filename, char *sem1, char *sem2){
    int i=0;
    char count[5],size[5],csize[5],tsize[5];

    sprintf(size,"%d",barg);
    sprintf(csize,"%d",carg);
    sprintf(tsize,"%d",targ);

    for(;i<varg;++i){
        sprintf(count,"%d",i);
        createAProcess("vaccinator", (char*[]){size,csize,tsize,sem1,sem2,count,NULL});
    }
}

void createCitizen(int carg,int targ,int barg){
    pid_t pid;
    int i=0;
    char count[5],tsize[5],size[5];

    sprintf(size,"%d",barg);
    sprintf(tsize,"%d",targ);

    for(;i<carg;++i){
        sprintf(count,"%d",i); 
        pid = createAProcess("citizen", (char*[]){count,tsize,size,NULL});
        writeSharedMem2("Citizens",carg,targ,pid);
    }
}

void writeSharedMem2(const char *sharedName, int carg, int targ, pid_t pid){
    void *addr;
    int fd,i=0;
    struct Citizen *citizens;
    int size = carg * (sizeof(struct Citizen));

    openFD(&fd,sharedName,size);

    addr = mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 214\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 217");
    }

    citizens = (struct Citizen*)addr; 

    while(citizens[i].pid != 0){
        //printf("already added: %d - %d - %s\n",processes[i].pid, processes[i].numberOfSwitches, processes[i].fifoName);
        ++i;
    }

    citizens[i].pid = pid;
    citizens[i].remaining = targ;
}

int allCitizensFinished(const char *sharedName, int carg){
    void *addr;
    int fd,i=0,flag=1;
    struct Citizen *citizens;
    int size = carg * (sizeof(struct Citizen));

    openFD(&fd,sharedName,size);

    addr = mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED){
        printErrorExit("Error: Mmap in line 214\n");
    }
    if(close(fd) == -1){
        printErrorExit("Error: Close in line 217");
    }

    citizens = (struct Citizen*)addr; 

    while(citizens[i].pid != 0){
        if(citizens[i].remaining > 0){
            flag=0;
            break;
        }
        //printf("already added: %d - %d - %s\n",processes[i].pid, processes[i].numberOfSwitches, processes[i].fifoName);
        ++i;
    }

    return flag;
}

void killVaccinators(){
    int i=0;
    while(vaccinators[i] != 0){
        kill(vaccinators[i],SIGKILL);
        i++;
    }
}