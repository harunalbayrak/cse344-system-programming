#include<unistd.h>
#include<string.h>
#include<stdlib.h>

void printErrorExit(const char *errMessage){
    write(STDERR_FILENO,errMessage,strlen(errMessage));
    _exit(EXIT_FAILURE);
}

void printWithNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
    write(STDOUT_FILENO,"\n",1);
}

void printWithoutNewline(const char *message){
    write(STDOUT_FILENO,message,strlen(message));
}

int myStrcpy(char *str1, const char *str2){
    if(str2 == NULL){
        return -1;
    }

    while(*str2){
        *str1 = *str2;
        str1++;
        str2++;
    }

    *str1 = '\0';
    return 0;
}