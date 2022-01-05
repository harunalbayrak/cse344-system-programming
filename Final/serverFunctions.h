#include "linkedList.h"

#define BUFFER_SIZE 100000
#define MAX_COL_LEN 50
#define MAX_STRING_LEN 50
#define MAX_PATH_LEN 128
#define MAX_PORT_LEN 16

typedef enum QueryType { SELECT,UPDATE } QueryType;

typedef struct ServerArguments{
    char port[MAX_PORT_LEN];
    char pathToLogFile[MAX_PATH_LEN];
    int poolSize;
    char datasetPath[MAX_PATH_LEN];
} ServerArguments;

typedef struct Query{
    QueryType type;
    char selectedColumns[MAX_COL_LEN][MAX_STRING_LEN];
    int selectedLength;

    // type == SELECT
    bool isDistinct;
    bool isAllColumn;

    // type == UPDATE
    char updatedColumns[MAX_COL_LEN][MAX_STRING_LEN];
    char whereColumn[MAX_STRING_LEN];
    char whereValue[MAX_STRING_LEN];
} Query;

typedef struct Message{
    char buffer[BUFFER_SIZE];
    double seconds;
    int records;
} Message;

int becomeDaemon(int argc, char **argv);
void catchSIGINT(int sigNum);
void initialErrorCheck(int argc,char **argv);
void server_handleCommandline(int argc,char **argv);
void server_readCSV();
void server_output(double time, long data);
void server_openSocket();
void server_createPoolThreads();
void *threadFunction(void *arg);
void server_createAMessage(Node *readerColumnsHead, Node **readerRowsHead, Message *message, int count, int totalRecords, long time, bool isAll);
void server_reader(const Query *query, Node **readerColumnsHead, Node ***readerRowsHead, int *count, int *totalRecords);
void server_writer(const Query *query, Message *message);
int server_readerAccessDB(const Query *query, Node **readerColumnsHead, Node ***readerRowsHead, int *totalRecords);
int server_updateDB(const Query *query, int *totalRecords, Message *message);
void server_parseQuery(char *query, Query *_query);
void server_freeColumns();