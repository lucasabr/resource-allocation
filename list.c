/**
 *  list.c
 *
 *  Full Name:
 *  Course section:
 *  Description of the program: 
 *      Various list operations -- A modification of the Queue I used for Assignment 1
 *      This list can be used with requests (each process has its own list with its instructions)
 *      Or can be used with processes (ie: blocked_list in banker.c)
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "process.h"



// add a new node to the list 
void insert(List *list, Request *request) {
    // add the new process to the list 
    struct node *newNode = malloc(sizeof(struct node));

    newNode->request = request;

    //Checks if the tail exists, if it does point the tail to the newly added node
    //If the tail doesn't exist, then the head also doesn't exist and set the head to the newly added node
    if(list->tail!=NULL){
        list->tail->next = newNode;
    }
    else{
        list->head= newNode;
    }
    list->size++;
    //Sets tail to newly added node
    list->tail = newNode;
}

//inserts a process
void insertProc(List *list, Process *proc){
    // add the new process to the list 
    struct node *newNode = malloc(sizeof(struct node));

    newNode->process = proc;

    //Checks if the tail exists, if it does point the tail to the newly added node
    //If the tail doesn't exist, then the head also doesn't exist and set the head to the newly added node
    if(list->tail!=NULL){
        list->tail->next = newNode;
    }
    else{
        list->head= newNode;
    }
    list->size++;
    //Sets tail to newly added node
    list->tail = newNode;
}


// delete the selected now from the list
void delete(List *list, Process *proc) {
    Node *temp = list->head;
    Node *prev = NULL;
    while(temp!=NULL){
        if(temp->process->pid == proc->pid){
            if(temp == list->head){
                list->head = temp->next;
            }
            else if(temp == list->tail){
                list->tail = prev;
            }
            else {
                prev->next = temp->next;
            }
            list->size--;
            if(list->size == 0) {
                list->tail = NULL;
                list->head = NULL;
            }
        }
        prev = temp;
        temp = temp->next;
    }
}
Request getNextRequest(List *list, Request *request){
    Node *temp = list->head;
    Request *next = NULL;
    while(temp!=NULL){
        if(temp->request->task_number == request->task_number){
            if(temp->request->request_id == request->request_id+1){
                next = temp->request;
            }
        }
        temp = temp->next;
    }
    return (*next);
}

//Shows the list (mainly for debugging purposes)
void toString(List *list){
    Node *temp = list->head;
    printf("Size %d ", list->size);
    while(temp!=NULL){
        printf("node %d %d %d %d %d ", temp->request->type, temp->request->task_number, temp->request->resource_type, temp->request->resource_count, temp->request->request_id);
        temp = temp->next;
    }
    printf("\n");
}