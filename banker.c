/**
 *  banker.c
 *
 *  Full Name: Lucas Aguiar
 *  Course section: M
 *  Description of the program: Executes FIFO and Banker Algorithms for Resource Allocation On a specific input
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "request.h"
#include "process.h"
#include "resource.h"

#define SIZE    100
enum request_types{initiate, request, release, compute, terminate};
enum status{ready, blocked, computing, terminated, aborted};

//Returns true if the system is in a deadlocked state, otherwise return false. Used for the FIFO algorithm
//System is in a deadlocked state if all the not finished processes are in a blocked state.
bool detect_deadlock(int num_of_processes, Process *proccesses){
	bool deadlock = true;
	for(int i=0; i<num_of_processes; i++){
		if(proccesses[i].status == ready || proccesses[i].status == computing){
			deadlock = false;
		}
	}
	return deadlock;
}

//Code for a fifo request instruction
void fifo_request(List *list, int num_of_resources, int num_of_processes, Request *req, Process *proc, Resource *resource, int alloc_table[num_of_processes][num_of_resources]){
	//Grants request if the resource has sufficient units available
	//Increments alloc table and the resources quantity allocated 
	if(resource->quantity >= resource->quantity_allocated+req->resource_count){
		alloc_table[proc->pid-1][req->resource_type-1] += req->resource_count;
		resource->quantity_allocated += req->resource_count;
		//If the request was from a blocked state, sets process to ready and deletes from the blocked list
		if(proc->status == blocked) {
			proc->status = ready;
			delete(list, proc);
		}
	}
	//If it cannot grant request increment time blocked, and if process isn't already blocked, set to blocked and inserts into blocked list
	else {
		if(proc->status != blocked){
			insertProc(list, proc);
			proc->status = blocked;
			proc->can_run = 0;
		}
		proc->time_blocked++;
	}
}


//Shared function between FIFO and Banker Algorithms to execute Release Instruction -- decrements from alloc table and resources quantity allocated
void fifo_banker_release(int num_of_resources, int num_of_processes, Request *req, Resource *resource, int alloc_table[num_of_processes][num_of_resources]){
	alloc_table[req->task_number-1][resource->resource_num-1] -= req->resource_count;
	resource->quantity_allocated -= req->resource_count;
}

//Code for a Banker initiate instruction
void banker_initiate(int *num_terminated, int quantity, Process *proc, Resource *resource, int num_of_resources, int num_of_processes, int max_claims[num_of_processes][num_of_resources]){
	//If claim is greater than the resources units, aborts the process and outputs a detailed explanation; Otherwise sets max_claim to the correct value
	if(quantity > resource->quantity) {
		(*num_terminated)++;
		proc->bstatus = aborted;
		printf("Banker aborts task %d before run begins:\n\t claim for resource %d (%d) exceeds number of units present (%d)\n", proc->pid, resource->resource_num, quantity, resource->quantity);
	}
	else {
		max_claims[proc->pid-1][resource->resource_num-1] = quantity;
	}
}

//Returns true if its a safe state, false otherwise; used for the bankers request instruction
bool isSafeState(Request *req, Process *processes, Resource *resources, int num_of_resources, int num_of_processes, int max_claims[num_of_processes][num_of_resources], int alloc_table[num_of_processes][num_of_resources]) {
	int temp_alloc_table[num_of_processes][num_of_resources];
	int resource_remaining[num_of_resources];
	int finished = 0;
	//recreate alloc_table with new value
	for(int i=0; i<num_of_processes; i++){
		if(processes[i].bstatus > computing) finished++;
		for(int j=0; j<num_of_resources; j++){
			if(i+1 == req->task_number && j+1 == req->resource_type) temp_alloc_table[i][j] = alloc_table[i][j] + req->resource_count;
			else temp_alloc_table[i][j] = alloc_table[i][j];
		}
	}
	//recreate resources remaining with new value
	for(int j=0; j<num_of_resources; j++){
		if(j+1 == req->resource_type) resource_remaining[j] = resources[j].quantity - resources[j].quantity_allocated - req->resource_count;
		else resource_remaining[j] = resources[j].quantity - resources[j].quantity_allocated;
	}	
	if(resource_remaining[req->resource_type-1] < 0){
		return false;
	}
	bool safe_state = true;
	bool can_finish = true;
	//loops through unfinished processes, as all need to be able to finish (in some order) in order for a state to be safe
	for(int i=0; i<num_of_processes-finished; i++){
		safe_state = false;
		//Loops through all processes and look for a process where the banker can satisfy its max claims with the current resource. If it cannot find anything in this loop, safe state
		//If it cannot find anything in this loop, safe state will be false, and the function will return false. Otherwise will continue to loop i times
		for(int j=0; j<num_of_processes; j++){
			if(processes[j].assumed_done == 0){
				if(processes[j].bstatus > computing) {
					processes[j].assumed_done = 1;
				}
				else {
					can_finish = true;
					//Loops through all resources and if for one of the resources, the banker cannot satisfy the max claim, sets can finish to false
					for(int k=0; k<num_of_resources; k++){	
						if(resource_remaining[k] < max_claims[j][k] - temp_alloc_table[j][k]){
							can_finish = false;
						}
					}
					//If can finish is true, that means the process can be terminated and we pretend it terminates, using assumed_done, resource_remaining and temp_alloc_table
					//Breaks out of the loop as a terminating process is found and goes through the outer loop once more as all unfinished processes need to be able to terminate
					if(can_finish){
						safe_state = true;
						processes[j].assumed_done = 1;
						for(int k=0; k<num_of_resources; k++){	
							resource_remaining[k] += temp_alloc_table[j][k];
							temp_alloc_table[j][k] = 0;
						}
						j=num_of_processes;
					}
				}			
			}
		}
		//if state is unsafe, exit out of outer loop
		if(!safe_state) {
			i = num_of_processes;
		}
	}
	//reset assumed_done
	for(int i=0; i<num_of_processes; i++) {
		processes[i].assumed_done = 0;
	}
	return safe_state;
}

//Code for bankers request instruction
void banker_request(List *list, int *num_terminated, Request *req, Process *processes, Resource *resources, int num_of_resources, int num_of_processes, int max_claims[num_of_processes][num_of_resources], int alloc_table[num_of_processes][num_of_resources]){
	//If the request would give the process more than its max claim, output detailed explanation and abort the process
	if(req->resource_count+alloc_table[req->task_number-1][req->resource_type-1] > max_claims[req->task_number-1][req->resource_type-1]) {
		printf("Banker aborts task %d during execution:\n\t request for resource %d (%d) exceeds max claim (%d)\n", req->task_number, req->resource_type, req->resource_count + alloc_table[req->task_number-1][req->resource_type-1], max_claims[req->task_number-1][req->resource_type-1]);
		(*num_terminated)++;
		processes[req->task_number-1].bstatus = aborted;
	}
	//Otherwise check if the process is in a safe state with this corresponding request
	else {
		//if it is, increment alloc table and resources quantity allocated
		if(isSafeState(req, processes, resources, num_of_resources, num_of_processes, max_claims, alloc_table)){
			alloc_table[req->task_number-1][req->resource_type-1] += req->resource_count;
			resources[req->resource_type-1].quantity_allocated += req->resource_count;
			//if the process is in a blocked state, set it to ready and remove from the blocked list
			if(processes[req->task_number-1].bstatus == blocked) {
				processes[req->task_number-1].bstatus = ready;
				delete(list, &processes[req->task_number-1]);
			}
		}
		//If its not in a safe state, increment banker blocked, and if it isnt already in a blocked state, set it to blocked and add it to the blocked list
		else {
			if(processes[req->task_number-1].bstatus != blocked){
				insertProc(list, &processes[req->task_number-1]);
				processes[req->task_number-1].bstatus = blocked;
				processes[req->task_number-1].can_run = 0;
			}
			processes[req->task_number-1].banker_blocked++;
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int num_terminated = 0;
	int num_of_processes, num_of_resources;
	fp  = fopen(argv[1],"r");
	//Reads the # of processes and resources, and mallocs an array for the size of each
	fscanf(fp, "%d", &num_of_processes);    
	fscanf(fp, "%d", &num_of_resources);  
	
	Resource *resources = malloc(num_of_resources * sizeof(Resource));
	Process *processes = malloc(num_of_processes * sizeof(Process));
	int request_number[num_of_processes];
	//Reads the quantity of each process/
	for(int i=0; i<num_of_resources; i++){
		fscanf(fp, "%d", &resources[i].quantity);
		resources[i].resource_num = i+1;
		resources[i].quantity_allocated = 0;
	}

	Request *currentRequests = malloc(num_of_processes * sizeof(Request));

	int max_claims[num_of_processes][num_of_resources];
	int alloc_table[num_of_processes][num_of_resources];
	//Initializes processes array, as well as max_claims and alloc_table 
	for (int i=0; i<num_of_processes; i++){

		currentRequests[i].type = -1;
		processes[i].pid = i+1;
		processes[i].status = ready;
		processes[i].bstatus = ready;
		processes[i].time_blocked = 0;
		processes[i].time_to_compute = 0;
		processes[i].can_run = 1;
		processes[i].assumed_done = 0;
		request_number[i] = 0;
		for(int j=0; j<num_of_resources; j++){
			max_claims[i][j] = 0;
			alloc_table[i][j] = 0;
		}
	}

	int clock_time = 0;
	//Sets up a list for each process, reads each instruction and adds it to its corresponding list
	List *list = malloc(num_of_processes * sizeof(List));
	while(num_terminated!=num_of_processes){
		char request_type[10];
		Request *r = malloc(sizeof(Request));
		fscanf(fp, "%s %d %d %d",
						request_type,
						&r->task_number,
						&r->resource_type,
						&r->resource_count);
		r->request_id = request_number[r->task_number-1];
		request_number[r->task_number-1]++;
		if(strcmp(request_type, "initiate") == 0){
			r->type = initiate;
			if(currentRequests[r->task_number-1].type == -1){
				currentRequests[r->task_number-1] = (*r);
			}		
		}
		else if(strcmp(request_type, "request") == 0){
			r->type = request;
		}
		else if(strcmp(request_type, "release") == 0){
			r->type = release;
		}
		else if(strcmp(request_type, "compute") == 0){
			r->type = compute;
		}
		else if(strcmp(request_type, "terminate") == 0){
			r->type = terminate;
			num_terminated++;
		}
		insert(&list[r->task_number-1], r);
	}
	num_terminated = 0;


	List *blocked_list = malloc(sizeof(List));
	int blocked_size;
	//Runs FIFO Algorithm
	//Loops till all processes terminate
	while(num_terminated!=num_of_processes){
		//Deals with blocked processes first
		//Uses a blocked list that uses FIFO to check in the order that the processes blocked
		blocked_size = blocked_list->size; 
		Node *temp = blocked_list->head;
		for(int i = 0; i<blocked_size; i++){
			fifo_request(blocked_list, num_of_resources, num_of_processes, &currentRequests[temp->process->pid-1], temp->process, &resources[currentRequests[temp->process->pid-1].resource_type-1], alloc_table);	
			temp = temp->next;		
		}
		//Loops through processes and deals with computations/new instructions for ready/computing processes
		//Loops through twice, first for initiating/requesting/computing instructions, then for releasing/terminating instructions
		//This is because the released resources can only be available next clock cycle, so releasing them last will guarantee that
		for(int i=0; i<num_of_processes; i++){
			Request current  = currentRequests[i];
			if(processes[i].status == ready){
				if(processes[i].can_run == 1){
					switch(current.type){
						case initiate:
							//does nothing because max claims are ignored for FIFO algorithm
							break;
						case request:
							fifo_request(blocked_list, num_of_resources, num_of_processes, &current, &processes[i], &resources[current.resource_type-1], alloc_table);
							break;
						case compute:
							processes[i].time_to_compute = current.resource_type;
							processes[i].status = computing;
							break;
					}
				}
			}
			//Decrements from computing processes
			if(processes[i].status == computing){
				processes[i].time_to_compute--;
				if(processes[i].time_to_compute == 0) {
					processes[i].status = ready;
				}
			}
		}
		//Loops specifically for executing releasing and terminating instructions
		for(int i=0; i<num_of_processes; i++){
			Request current  = currentRequests[i];
			if(processes[i].status == ready){
				//Setting can_run to 1 ensures that blocked processes won't run 2 instructions in 1 cycle. If their initial blocked instruction runs, it will be set to ready and go
				//Through the first loop, and execute the same instruction again. This ensures that won't happen
				if(processes[i].can_run == 0){	
					processes[i].can_run = 1;
				}
				else {
					switch(current.type){
						case release: 
							fifo_banker_release(num_of_resources, num_of_processes, &current, &resources[current.resource_type-1], alloc_table);
							break;
						case terminate: 
							num_terminated++;
							processes[i].status = terminated;
							processes[i].finishing_time = clock_time;
							for(int j=0; j<num_of_resources; j++){
								resources[j].quantity_allocated -= alloc_table[i][j];
								alloc_table[i][j] = 0;
							}
							break;
					}
					
				}
				//if the process is in a ready state, get its next instruction (for the next clock cycle)
				if(processes[i].status == ready) currentRequests[i] = getNextRequest(&list[i], &current);
			}
		}
		//increments clock time
		clock_time++;
		//If the system is in a deadlocked state, recovers from it. Aborts unfinished process will lowest PID, and checks if the system is no longer deadlocked
		//If it still is deadlocked, then it aborts the next lowest PID process
		//If it is not deadlocked after aborting a process, it exits the loop and continues running
		if(detect_deadlock(num_of_processes, processes)){
			for(int i=0; i<num_of_processes; i++){
				if(processes[i].status == blocked) {
					processes[i].status = aborted;
					delete(blocked_list, &processes[i]);
					num_terminated++;
					for(int j=0; j<num_of_resources; j++){
						resources[j].quantity_allocated -= alloc_table[i][j];
						alloc_table[i][j] = 0;
					}
					for(int k=0; k<num_of_processes; k++){
						if(processes[k].status == blocked) {
							Request *current = &currentRequests[k];
							if(resources[current->resource_type-1].quantity >= resources[current->resource_type-1].quantity_allocated+current->resource_count){
								k = num_of_processes;
								i = num_of_processes;
							}
						}
					}
				}
			}
		}
		

	}


	//Reset Some variables
	num_terminated = 0;
	clock_time = 0;
	for(int i=0; i<num_of_processes; i++){
		currentRequests[i] = (*list[i].head->request);
		processes[i].can_run = 1;
		processes[i].time_to_compute = 0;
	}


	//Runs Banker Algorithm
	//Loops till all processes terminate (or abort)
	while(num_terminated!=num_of_processes){
		//Deals with blocked processes first
		//Uses a blocked list that uses FIFO to check in the order that the processes blocked
		blocked_size = blocked_list->size; 
		Node *temp = blocked_list->head;
		for(int i = 0; i<blocked_size; i++){
			Request current = currentRequests[temp->process->pid-1];
			banker_request(blocked_list, &num_terminated, &current, processes, resources, num_of_resources, num_of_processes, max_claims, alloc_table);
			temp = temp->next;		
		}
		//Loops through processes and deals with computations/new instructions for ready/computing processes
		//Loops through twice, first for initiating/requesting/computing instructions, then for releasing/terminating instructions
		//This is because the released resources can only be available next clock cycle, so releasing them last will guarantee that
		for(int i=0; i<num_of_processes; i++){
			Request current  = currentRequests[i];
			if(processes[i].bstatus == ready){
				if(processes[i].can_run == 1){
					switch(current.type){
						case initiate:
							banker_initiate(&num_terminated, current.resource_count, &processes[current.task_number-1], &resources[current.resource_type-1], num_of_resources, num_of_processes, max_claims);
							break;
						case request:
							banker_request(blocked_list, &num_terminated, &current, processes, resources, num_of_resources, num_of_processes, max_claims, alloc_table);
							break;
						case compute:
							processes[i].time_to_compute = current.resource_type;
							processes[i].bstatus = computing;
							break;
					}
				}
			}
			//Decrements from computing processes
			if(processes[i].bstatus == computing){
				processes[i].time_to_compute--;
				if(processes[i].time_to_compute == 0) {
					processes[i].bstatus = ready;
				}
			}
		}
		//Loops specifically for executing releasing and terminating instructions
		for(int i=0; i<num_of_processes; i++){
			Request current  = currentRequests[i];
			if(processes[i].bstatus == ready){
				//Setting can_run to 1 ensures that blocked processes won't run 2 instructions in 1 cycle. If their initial blocked instruction runs, it will be set to ready and go
				//Through the first loop, and execute the same instruction again. This ensures that won't happen
				if(processes[i].can_run == 0){	
					processes[i].can_run = 1;
				}
				else {
					switch(current.type){
						case release: 
							fifo_banker_release(num_of_resources, num_of_processes, &current, &resources[current.resource_type-1], alloc_table);
							break;
						case terminate: 
							num_terminated++;
							processes[i].bstatus = terminated;
							processes[i].banker_finishing = clock_time;
							for(int j=0; j<num_of_resources; j++){
								resources[j].quantity_allocated -= alloc_table[i][j];
								alloc_table[i][j] = 0;
							}
							break;
					}
					
				}
				//if the process is in a ready state, get its next instruction (for the next clock cycle)
				if(processes[i].bstatus == ready) currentRequests[i] = getNextRequest(&list[i], &current);
			}
			//If a process is aborted, free up all its resources
			//This cannot be done when processes get set to aborted, as the resources need to only become available during the next cycle, meaning 
			//They need to be free'd at the end
			else if(processes[i].bstatus == aborted) {
				for(int j=0; j<num_of_resources; j++){
					resources[j].quantity_allocated -= alloc_table[i][j];
					alloc_table[i][j] = 0;
				}
			}
		}
		clock_time++;

	}
	



	//Code for printing out each tasks stats (and incrementing total stats)
	printf("\tFIFO\t\t  BANKER'S\n");
	int fifo_total_waiting = 0, fifo_total_cycles = 0, banker_total_waiting = 0 , banker_total_cycles = 0;
	for(int i=0; i<num_of_processes; i++) {
		if(processes[i].status == aborted) printf("Task %d\taborted   \t", i+1);
		else {
			printf("Task %d\t%d  %d  %0.f%%\t", i+1, processes[i].finishing_time, processes[i].time_blocked, (((double)processes[i].time_blocked)/processes[i].finishing_time)*100);
			fifo_total_cycles += processes[i].finishing_time;
			fifo_total_waiting += processes[i].time_blocked;
		}
		if(processes[i].bstatus == aborted) printf("Task %d\taborted   \n", i+1);
		else {
			printf("Task %d\t%d  %d  %0.f%%\n", i+1, processes[i].banker_finishing, processes[i].banker_blocked, (((double)processes[i].banker_blocked)/processes[i].banker_finishing)*100);
			banker_total_cycles += processes[i].banker_finishing;
			banker_total_waiting += processes[i].banker_blocked;
		}
	}
	//Prints out total stats
	printf("total \t%d  %d  %0.f%%\t", fifo_total_cycles, fifo_total_waiting, (((double)fifo_total_waiting)/fifo_total_cycles)*100);
	printf("total\t%d  %d  %0.f%%\n", banker_total_cycles, banker_total_waiting, (((double)banker_total_waiting)/banker_total_cycles)*100);



	fclose(fp);

	return 0;
}
