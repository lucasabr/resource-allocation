/**
 *  request.h
 *
 *  Full Name: Lucas Aguiar
 *  Course section: M 
 *  Representation of an instruction in the system
 *  
 */


#ifndef REQUEST_H
#define REQUEST_H

// representation of a a process
typedef struct request {
    int  type;
    int  task_number;
    int  resource_type;  
    int  resource_count;
    int  request_id;
} Request;

#endif