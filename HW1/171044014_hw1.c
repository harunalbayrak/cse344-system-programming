#include"171044014_hw1.h"

int main(int argc , char** argv) {
    signal(SIGINT, catchCtrlC);

    initialErrorChecks(argc,argv);
    handleCommandLine(argc,argv);
    
    return 0;
}