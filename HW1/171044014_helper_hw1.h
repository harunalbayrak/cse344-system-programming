#include<unistd.h>

int myStrlen(const char *str){
    int len = 0;
    while(*str){
        len++;
        str++;
    }
    return len;
}

int toLowerChar(char ch){
    if(ch >= 'A' && ch <= 'Z'){
        return (ch+32);
    } else{
        return ch;
    }
}

void printErrorExit(const char *errMessage){
    write(STDERR_FILENO,errMessage,myStrlen(errMessage));
    _exit(1);
}

void printWithNewline(const char *message){
    write(STDOUT_FILENO,message,myStrlen(message));
    write(STDOUT_FILENO,"\n",1);
}

void printWithoutNewline(const char *message){
    write(STDOUT_FILENO,message,myStrlen(message));
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

int myStrcat(char *str1, const char *str2){
    if(str2 == NULL){
        return -1;
    }

    while(*str1){
        str1++;
    }

    while(*str2){
        *str1 = *str2;
        str1++;
        str2++;
    }

    *str1 = '\0';
    return 0;
}

int isNumber(const char *str){
    while(*str){
        if(*str >= '0' && *str <= '9'){
            str++;
        } else {
            return -1;   
        }
    }
    return 1;
}

void initialErrorChecks(int argc , char** argv){
    if(argc < 5){
        printErrorExit("Insufficient parameters: The program does not satisfy parameters.\n");
    }
    if(argc > 13){
        printErrorExit("Too many parameters: The program does not satisfy parameters.\n");
    }
}

void getNewPath(char *path,const char *d, const char *d2){
    myStrcpy(path, d);
    myStrcat(path, "/");
    myStrcat(path, d2);
}

void printByDepth(int depth){
    int i;

    for(i=0;i<depth+1;++i){
        if(i == 0)
            printWithoutNewline("|");
        printWithoutNewline("--");
    }
}

int compareString(const char *c1, const char *c2){
    while(*c1){
        if(*c1 != *c2){
            return -1;
        }
        c1++;
        c2++;
    }
    return 0;
}

int compareStringCaseinsensitive(const char *c1, const char *c2){
    while(*c1){
        if(toLowerChar(*c1) != toLowerChar(*c2)){
            return -1;
        }
        c1++;
        c2++;
    }
    return 0;
}

int compareStringRegex(const char *c1, const char *c2){
    int count = 0;

    if(*c2 == '+')
        return -1;
    while(*c1){
        if(toLowerChar(*c1) != toLowerChar(*c2)){
            if(*c2 != '+'){
                return -1;
            } else {
                if((toLowerChar(*c1) == toLowerChar(*(c2-1))) || (toLowerChar(*c1) == toLowerChar(*(c2+1)))){
                    if(toLowerChar(*c1) == toLowerChar(*(c2-1))){
                        c2--;
                    } else{
                        c2++;
                    }
                } else{
                    return -1;
                }
            }
        }
        c1++;
        c2++;
    }
    if(*c2 == '+'){
        while(*c2){
            count++;
            c2++;
        }
        return count > 1 ? -1 : 0;
    }
    if(*c1 == *c2){
        return 0;
    }
    return -1;
}



int charArrayToInteger(const char *str){
    int number = 0;
    int i = myStrlen(str), j = 0;
    int cc = 1;

    while(*str){
        cc = 1;
        for(j=0;j<i-1;++j){
            cc *= 10;
        }
        switch (*str){
            case '0':
                number += cc*0;
                break;
            case '1':
                number += cc*1;
                break;
            case '2':
                number += cc*2;
                break;
            case '3':
                number += cc*3;
                break;
            case '4':
                number += cc*4;
                break;
            case '5':
                number += cc*5;
                break;
            case '6':
                number += cc*6;
                break;
            case '7':
                number += cc*7;
                break;
            case '8':
                number += cc*8;
                break;
            case '9':
                number += cc*9;
                break;
            default:
                break;
        }

        str++;
        i--;
    }

    return number;
}