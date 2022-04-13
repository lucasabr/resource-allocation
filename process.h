/**
 *  process.h
 *
 *  Full Name: Lucas Aguiar
 *  Course section: M
 *  Representation of a process in the system.
 *  
 */


#ifndef PROCESS_H
#define PROCESS_H


// representation of a a process
typedef struct process {
    int  pid;
    int  status;
    int  bstatus;
    int  finishing_time;
    int  banker_finishing;
    int  time_blocked;
    int  banker_blocked;
    int  time_to_compute;
    int  can_run;
    int  assumed_done;
} Process;

#endif
