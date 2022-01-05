#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include"171044014_helper_hw2.h"

sig_atomic_t sigusr1_count = 0;
sig_atomic_t sigusr2_count = 0;
pid_t childrenPid[8];

void handler(int signal_number){
    if(signal_number == SIGUSR1)
        ++sigusr1_count;
    else if(signal_number == SIGUSR2)
        ++sigusr2_count;
    else if(signal_number == SIGINT){
        printErrorExit("Exiting the program... (Because of CTRL+C interrupt)\n");
        fflush(stdout);
        fflush(stderr);
    }
}

int handleProcesses(int fd);
int work(int fd, int index, int work);
int calculate(int fd, int work, int which);
int handleLine(int fd, int lineNum, double array1[], double array2[], double *x_7, double *y_7);
int handleString(char *str, double array1[], double array2[], double *x_7, double *y_7);
double calculateWithDegree(double val, double array[], int work, int degree);
double calculateAverageError(int fd, int which);
int writeEOL(int fd, int lineNum, double val);

int main(int argc, char **argv){
    if(argc != 2){
        printErrorExit("Error: Wrong argument number\n");
    }

    signal(SIGINT, handler);

    int fd = open(argv[1], O_RDWR, 0777);

    if(fd == -1){
        return -1;
    }

    handleProcesses(fd);

    close(fd);

    return 0;
}

int handleProcesses(int fd){
    double error5,error6;
    int i=0,j=0;
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask, emptyMask2;
    struct sigaction sa;

    setbuf(stdout, NULL);
    sigemptyset(&blockMask);
    sigaddset(&blockMask,SIGUSR1);
    sigaddset(&blockMask,SIGUSR2);

    if(sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1){
        printErrorExit("Error: Sigprocmask is failed\n");
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if(sigaction(SIGUSR1, &sa, NULL) == -1){
        printErrorExit("Error: Sigaction (SIGUSR1) is failed\n");
    }

    if(sigaction(SIGUSR2, &sa, NULL) == -1){
        printErrorExit("Error: Sigaction (SIGUSR2) is failed\n");
    }

    for(;i<8;++i){
        switch (childPid = fork()){
            case -1:
                printErrorExit("Error: The process is not created\n");
                break;

            case 0:
                childrenPid[i] = childPid;
                work(fd,i,0);

                if(kill(getppid(),SIGUSR1) == -1){
                    printErrorExit("Error: kill system call\n");
                }

                sigemptyset(&emptyMask2);
                if(sigsuspend(&emptyMask2) == -1 && errno != EINTR){
                    printErrorExit("Error: Sigsuspend is not provided\n");
                }

                if(sigusr2_count == 1){
                    work(fd,i,1);

                    _exit(EXIT_SUCCESS);
                }

                _exit(EXIT_SUCCESS);

            default:
                sigemptyset(&emptyMask);
                if(sigsuspend(&emptyMask) == -1 && errno != EINTR){
                    printErrorExit("Error: Sigsuspend is not provided\n");
                }

                if(sigusr1_count == 8){
                    error5 = calculateAverageError(fd,0);
                    printf("Error of polynomial of degree 5: %.1f\n",error5);
                    for(j=7;j>=0;--j){
                        kill(childrenPid[j],SIGUSR2);
                    }
                } 
                //printf("sigusr1: %d\n",sigusr1_count);
                while(sigusr1_count == 8){
                    childPid = wait(NULL);
                    if(childPid == -1){
                        if(errno == ECHILD){
                            error6 = calculateAverageError(fd,1);
                            printf("Error of polynomial of degree 6: %.1f\n",error6);
                            exit(EXIT_SUCCESS);
                        }else{
                            printErrorExit("wait");
                        }
                    } 
                }

                break;
        }
    }

    return 0;
}

int work(int fd, int index, int work){
    struct flock lock;
    
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        printErrorExit("Error: Filelock is not set");
    }

    calculate(fd,index,work);

    lock.l_type = F_UNLCK;
    if(fcntl(fd,F_SETLKW,&lock) == -1){
        printErrorExit("Error: Filelock is not set");
    }

    return 0;
}

int calculate(int fd, int work, int which){
    double array1[12];
    double array2[14];
    double x_7,y_7;
    double num5;
    double num6;

    lseek(fd,0,SEEK_SET);
    handleLine(fd,work,array1,array2,&x_7,&y_7);

    if(which == 0){
        num5 = calculateWithDegree(x_7,array1, work, 5);
        num5 = fabs(num5-y_7);        
        writeEOL(fd,work,num5);

    } else if(which == 1){
        num6 = calculateWithDegree(x_7,array2, work, 6);
        num6 = fabs(num6-y_7);
        writeEOL(fd,work,num6);
    }

    return 0;
}

int handleLine(int fd, int lineNum, double array1[], double array2[], double *x_7, double *y_7){
    int num = 0;
    char line[1024];
    char *str;

    if(fd == -1){
        return -1;
    }

    read(fd, line, sizeof(line)-1);
    str = line;
    strtok(str,"\n");

    while(str != NULL) {
        if(lineNum == num){
            handleString(str,array1,array2,x_7,y_7);
            return 0;
        }
        str = strtok(NULL, "\n");
        num++;
    }

    return -1;
}

int getLineFile(FILE *fp, int lineNum, double array1[], double array2[], double *x_7, double *y_7){
    char line[100];
    int num = 0;

    while(fgets(line, sizeof line, fp) != NULL){
        if(lineNum == num++){
            handleString(line,array1,array2,x_7,y_7);
            return 0;
        }
    }

    return -1;
}

int handleString(char *str, double array1[], double array2[], double *x_7, double *y_7){
    int i=0;
    char *string = strtok(str, ",");
    if(string == NULL || str == NULL){
        return -1;
    }

    while( string != NULL ) {
        if(i < 12)
            array1[i] = atof(string);
        if(i < 14)
            array2[i] = atof(string);
        if(i == 14)
            *x_7 = atof(string);
        if(i == 15)
            *y_7 = atof(string);

        string = strtok(NULL, ",");
        i++;
    }

    return 0;
}

double calculateWithDegree(double val, double array[], int work, int degree){
    int i=0,j=0;
    double total = 0;
    double number = 1.0;
    double number2 = 1.0;
    int length = (degree*2)+2;

    for(i=0;i<length;++i){
        number = 1.0;
        number2 = 1.0;
        for(j=0;j<length;++j){
            if(j != i && i % 2 == 0){
                if(j % 2 == 0){
                    number *= (val-array[j]);
                    number2 *= (array[i]-array[j]); 
                }
            }
        }
        if(i%2 == 0){
            if(degree == 6){
                if(i == 0){
                    printf("Polynomial %d: ",work);
                }
                if(i==length-2){
                    printf("%.1f\n",(number/number2));
                } else{
                    printf("%.1f,",(number/number2));
                }
            }
            total += (number/number2) * array[i+1];
        }
    }

    return total;
}

int writeEOL(int fd, int lineNum, double val){
    int num = 0,length=0;
    char line[1024];
    char *temp;
    char *temp2;
    char array[4];

    lseek(fd,0,SEEK_SET);
    read(fd, line, sizeof(line)-1);

    temp = malloc(strlen(line));
    myStrcpy(temp,line);
    temp = strtok(temp,"\n");
    temp2 = temp;

    while(temp != NULL) {
        if(num > 0)
            ++length;
        if(lineNum > num){
            length += strlen(temp);
        } else if(lineNum == num){
            length += strlen(temp);
            lseek(fd,length,SEEK_SET);
            sprintf(array, "%.1f", val);
            write(fd, ",",strlen(","));
            write(fd, array,strlen(array));
            write(fd, "\n",strlen("\n"));
        }
        else if(num > lineNum){
            write(fd,temp,strlen(temp));
            write(fd,"\n",strlen("\n"));
        }
        temp = strtok(NULL, "\n");
        num++;
    }

    free(temp);
    free(temp2);
    return -1;
}

double calculateAverageError(int fd, int which){
    char line[1024];
    char *str;
    int num=0;
    double result=0;

    if(fd == -1){
        return -1;
    }

    if(which != 0 && which != 1){
        return -1;
    }
    
    lseek(fd,0,SEEK_SET);
    read(fd, line, sizeof(line)-1);
    str = line;
    str = strtok(str,",");

    while(str != NULL) {
        if(num % (16+which) == 0 && num != 0){
            result += atof(str);
        }
        num++;
        str = strtok(NULL, ",");
    }

    result /= 8;

    return result;
}