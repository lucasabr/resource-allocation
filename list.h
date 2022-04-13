/**
 *  list.h
 *
 *  Full Name:
 *  Course section:
 *  Description of the program: list data structure containing the tasks in the system
 *  
 */
 
#include "request.h"
#include "process.h"

typedef struct node {
    Request *request;
    Process *process;
    struct node *next;
}Node;

typedef struct list {
    Node *head;
    Node *tail;
    int size;
}List;

// insert and delete operations.
void insert(List *list, Request *request);
void insertProc(List *list, Process *proc);
void delete(List *list, Process *proc);
void toString(List *list);
Request getNextRequest(List *list, Request *request);
