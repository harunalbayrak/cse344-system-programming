#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

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