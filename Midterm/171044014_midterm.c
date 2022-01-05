#include"171044014_func_midterm.h"
int main(int argc, char **argv){
    signal(SIGINT, catchCtrlC);

    initialErrorCheck(argc, argv);
    handleCommandLine(argc, argv);
}