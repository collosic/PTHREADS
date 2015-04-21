#ifndef PTHREADS
#define PTHREADS 

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>


// constants
#define MAXLINE 512
#define DATA_SIZE 100000

// constant strings
const char NEWLINE[2] = "\n";

// Structs
typedef struct {
    int t_id;
    char string_found[4];
    int slice_index;
    int string_index;
    int array_start;
    int array_end;
} t_data;

// function definitions
void generateData(FILE *ftpr, int *NT, int *NS);
void initThreadsData(int id, int start, int end, t_data *data);
void *searchString(void *data);
void writeToFile();
void releaseData();

// global variables used by the pthreads
char *dataArray[DATA_SIZE];
char *searchStr;
char out[MAXLINE];
pthread_mutex_t mutexstring;


#endif
