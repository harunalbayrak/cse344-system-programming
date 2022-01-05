#include "linkedList.h"

void push(Node **head,char data[]){
    Node *temp = (Node *) malloc(sizeof(Node));

    temp->data = (char *) malloc(strlen(data)+1);
    strcpy(temp->data,data);
    temp->next = (*head);
    temp->prev = NULL;

    if((*head) != NULL){
        (*head)->prev = temp;
    }

    (*head) = temp;
}

void deleteNode(Node **head, char data[]){
    Node *temp = *head, *prev;

    if(temp != NULL && (strcmp(temp->data,data) == 0)){
        *head = temp->next;
        free(temp);
        return;
    }

    while(temp != NULL && (strcmp(temp->data,data) != 0)){
        prev = temp;
        temp = temp->next;
    }

    if(temp == NULL)
        return;

    prev->next = temp->next;

    free(temp->data);
    free(temp);
}

void printLinkedList(Node *node){
    while(node != NULL){
        printf(" %s ",node->data);
        node = node->next;
    }
}

void printLLInt(Node *node, int x){
    int i=0;

    if(node == NULL)
        return;

    if(x < 0)
        printErrorExit("Error");

    while(node != NULL && i!=x){
        printf(" %s ",node->data);
        node = node->next;
        i++;
    }
}

void printLLReverseInt(Node *node, int x){
    int i=0;

    if(node == NULL)
        return;

    if(x < 0)
        printErrorExit("Error");

    while(node->next != NULL)
        node = node->next;

    while(i != x){
        if(node == NULL)
            break;
        printf(" %s ",node->data);
        node = node->prev;
        i++;
    }
}

void printLLReverseAtIndex(Node *node, int index){
    int i=0;

    if(node == NULL)
        return;

    if(index < 0)
        printErrorExit("Index Error");

    while(node->next != NULL)
        node = node->next;

    if(index == 0)
        printf(" %s ",node->data);

    while(i != index){
        if(node == NULL)
            break;

        node = node->prev;
        i++;
        if(i == index){
            printf(" %s ",node->data);
            return;
        }
    } 
}

char *AtIndex(Node *node, int index){
    int i=0;

    if(index < 0)
        printErrorExit("Index Error\n");

    while(node != NULL){
        if(i==index){
            return node->data;
        }
        node = node->next;
        i++;
    }

    return NULL;
}

char *AtIndexReverse(Node *node, int index){
    int i=0;

    if(node == NULL)
        return NULL;

    if(index < 0)
        printErrorExit("Index Error");

    while(node->next != NULL)
        node = node->next;

    if(index == 0)
        return node->data;

    while(i != index){
        if(node == NULL)
            break;

        node = node->prev;
        i++;
        if(i == index){
            return node->data;
        }
    } 

    return NULL;
}

int updateValue(Node *node, char *value, char *newValue){
    while(node != NULL){
        if(node->data != NULL){
            if(strcmp(value,node->data) == 0){
                free(node->data);

                node->data = (char *) malloc(strlen(newValue)+1);

                strcpy(node->data,newValue);
                return 1;
            }
        }
        node = node->next;
    }

    return 0;
}

int updateValuesAtIndexes(Node *node, int indexes[], int length, const char *newValue){
    int i=0,nn=0,x=0;

    while(node != NULL){
        if(node->data != NULL){
            nn = indexes[x];
            if(i==nn){
                free(node->data);

                node->data = (char *) malloc(strlen(newValue)+1);

                strcpy(node->data,newValue);
                
                x++;
                if(x == length){
                    return 1;
                }
            }
        }
        node = node->next;
        i++;
    }

    return 0;
}

int findForDistinct(Node *node, int rowCount, char *tok){
    int i=0;

    while(node != NULL){
        if(i == rowCount){
            break;
        }
        if(strcmp(node->data,tok)==0){
            return 0;
        }
        node = node->next;
        i++;
    }

    return 1;
}

void freeList(Node *head){
    Node *temp;

    while(head != NULL){
        temp = head;
        head = head->next;
        free(temp->data);
        free(temp);
    }
}