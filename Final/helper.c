#include "helper.h"

void printErrorExit(const char *errMessage){
    write(STDERR_FILENO,errMessage,strlen(errMessage));
    _exit(1);
}

void writeErrorExit(FILE *log, const char *errMessage){
    fprintf(log, "%s\n",errMessage);
    fflush(log);
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

int removeCommasInDelimiter(char line[],char del,int num){
    int x=0;
    int first=0,second=0,flag1=0;

    if(num+1 >= strlen(line)){
        return 0;
    }

    for(x=num;x<strlen(line);x++){
        if(line[x] == del && flag1 == 0){
            first = x;
            flag1 = 1;
            continue;
        }

        if(line[x] == del && flag1 == 1){
            second = x;
            break;
        }
    }
    
    for(x=first;x<second;x++){
        if(line[x] == ','){
            line[x] = '-';
        }
    }

    return second+1;
}