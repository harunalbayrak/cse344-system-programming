#ifndef CLIENTFUNCTIONS_H
#define CLIENTFUNCTIONS_H

#define BUFFER_SIZE 100000
#define MAX_PATH_LEN 128
#define MAX_PORT_LEN 16
#define MAX_IP_LEN 32

typedef struct ClientArguments{
    int id;
    char ipv4Address[MAX_IP_LEN];
    char port[MAX_PORT_LEN];
    char pathToQueryFile[MAX_PATH_LEN];
} ClientArguments;

typedef struct Message{
    char buffer[BUFFER_SIZE];
    double seconds;
    int records;
} Message;

void initialErrorCheck(int argc,char **argv);
void client_handleCommandline(int argc,char **argv);
void client_readQueryFile();
void client_sendQuery(char *line, int *count);
void client_handleQuery(char line[]);
void client_openSocket();
void client_sendToServer(char *message);

#endif