#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

void catchChild(int sig);
void catchCtrlC(int sig_num);
void whetherPositiveNumber(const char *arg, char val);
void whetherGreaterThan(int num1, int num2, char val);
void initialErrorCheck(int argc,char **argv);
void waitAllProcesses(int totalProcess);
void handleCommandLine(int argc, char **argv);
void unlinkResources(char *semaphoreNames[], int size);
int createAProcess(char *name, char *arg[]);
void createNurse(int narg, int barg, int targ, int carg, char *filename, char *sem1, char *sem2);
void createVaccinator(int varg, int barg, int carg, int targ, char *filename, char *sem1, char *sem2);
void createCitizen(int carg,int targ,int barg);
void writeSharedMem2(const char *sharedName, int carg, int targ, pid_t pid);
int allCitizensFinished(const char *sharedName, int carg);
void killVaccinators();