#include<stdio.h>
#include<stdlib.h>
#include "clientFunctions.h"

extern ClientArguments clientArguments;

int main(int argc, char **argv){
    
    initialErrorCheck(argc, argv);
    client_handleCommandline(argc, argv);
    client_openSocket();
    client_readQueryFile();

    return EXIT_SUCCESS;
}