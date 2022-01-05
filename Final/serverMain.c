#include<signal.h>
#include "serverFunctions.h"

int main(int argc, char **argv){
    signal(SIGINT, catchSIGINT);

    becomeDaemon(argc,argv);

    return EXIT_SUCCESS;
}