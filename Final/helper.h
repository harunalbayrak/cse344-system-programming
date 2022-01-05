#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

void printErrorExit(const char *errMessage);
void writeErrorExit(FILE *log, const char *errMessage);
void printWithNewline(const char *message);
void printWithoutNewline(const char *message);
int isPositiveNumber(const char *str);
int removeCommasInDelimiter(char line[],char del,int num);