#include<semaphore.h>

typedef struct student{
    char name[20];
    int quality;
    int speed;
    int price;
    int isAvailable;
    int done;
} student;

void catchCtrlC(int sig_num);
void initialErrorCheck(int argc,char **argv);
void checkFileIsValid(const char *filename);
void handleCommandLine(int argc,char **argv);
void exitTheFunc(int exitFlag, int numberOfStudents);
int howManyLine(const char *filename);
void handleAllStudents(const char *filename, student students[] ,int numberOfStudents);
void createStudentThreads(int number);
void selectStudent(char spec, student students[], int size);
void selectNum(int *selected, student students[], int size, char which);
int isAllUnavailable(int size);
void handleBuffer(char *buf, student students[], int size, int num);
int checkQueueValues(long size, int numberOfStudents);
void semaphoresInits(int numberOfStudents);
void semaphoresDestroys(int numberOfStudents);
void printSolvedHomeworks(int size);
void *threadStudent(void *arg);
void *threadH(void *arg);