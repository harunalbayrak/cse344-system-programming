#include "helper.h"

typedef struct Node{
    char *data;
    struct Node *next;
    struct Node *prev;
} Node;

// O(1)
void push(Node **head,char data[]);
void deleteNode(Node **head, char data[]);
void printLinkedList(Node *node);
void printLLInt(Node *node, int x);
void printLLReverseInt(Node *node, int x);
void printLLReverseAtIndex(Node *node, int index);
char *AtIndex(Node *node, int index);
char *AtIndexReverse(Node *node, int index);
int updateValue(Node *node, char *value, char *newValue);
int updateValuesAtIndexes(Node *node, int indexes[], int length, const char *newValue);
int findForDistinct(Node *node, int rowCount, char *tok);
void freeList(Node *head);