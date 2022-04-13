# makefile for scheduling program        
CC=gcc
DEPS = resource.h request.h list.h process.h
OBJ = banker.o list.o
CFLAGS=-Wall
STD_FLAG=-std=c99


banker.o: banker.c
	$(CC) $(CFLAGS) banker.c list.c $(STD_FLAG)  -o banker

list.o: list.c list.h process.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm -rf *.o    

