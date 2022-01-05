#include"171044014_func_hw4.h"
#include<signal.h>

int main(int argc, char **argv){
    signal(SIGINT, catchCtrlC);

    initialErrorCheck(argc, argv);
    handleCommandLine(argc, argv);
}