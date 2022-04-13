/**
 *  resource.h
 *
 *  Full Name: Lucas Aguiar
 *  Course section: M 
 *  Representation of a resource in the system
 *  
 */


#ifndef RESOURCE_H
#define RESOURCE_H

typedef struct resource {
    int  resource_num;
    int  quantity;
    int  quantity_allocated;  
} Resource;

#endif