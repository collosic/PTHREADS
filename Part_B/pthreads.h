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
#define FOUND_SIZE 4

// constant strings
const char NEWLINE[2] = "\n";

// Structs
typedef struct {
    int t_id;
    char string_found[FOUND_SIZE];
    int slice_index;
    int string_index;
    int array_start;
    int array_end;
} t_data;

// function definitions
void generateData(FILE *ftpr, int *NT, int *NS);
void initThreadsData(int id, t_data *data);
void *searchString(void *data);
void getSlice(int *current_slice, t_data *data);
void writeToFile();
void releaseData();

// global variables used by the pthreads
int data_size;
char **dataArray;
char *searchStr;
char out[MAXLINE];
int NS;
int slice_size;
int SV;
pthread_mutex_t mutexsv;
pthread_mutex_t mutexstring;


#endif
