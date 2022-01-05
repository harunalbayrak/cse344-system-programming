#include<unistd.h>
#include<string.h>

void printErrorExit(const char *errMessage){
    write(STDERR_FILENO,errMessage,strlen(errMessage));
    _exit(1);
}

void printWithNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
    write(STDOUT_FILENO,"\n",1);
}

void printWithoutNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
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

int countDigit(long number){
    int res = 0;
    while(number != 0){
        number /= 10;
        res++;
    }

    return res;
}