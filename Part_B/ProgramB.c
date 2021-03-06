/*  
 *  Christian Collosi
 *  11233529
 *  CS131
 *  Lab1 - Part B
 *	compiled with the following gcc command
 *	$ gcc -lpthread -std=c99 -Wall ProgramA.c
 *
 */

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


// Start of Main
int main(int argc, char *argv[])
{
    FILE *fptr = NULL;
    if (argc == 2) {
        fptr = fopen(argv[1], "r");
        if (fptr == NULL) {
            puts("unable to open file");
            return -1;
        }
    } else {
        puts("invalid argument");
        return -1;
    }
    
    int NT;
    // get all the needed data from the file and pul the file into an array
    generateData(fptr, &NT, &NS);
    fclose(fptr);

    // construct pthreads
    t_data thread_data[NT];
    pthread_t threads[NT]; 
    pthread_attr_t attr;
    void *status;
    int rc;
    long t;
    SV = 1;
    int extra = !(data_size % NS) ? 0 : NS - (data_size % NS);
    slice_size = (data_size + extra) / NS;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexsv, NULL);
    pthread_mutex_init(&mutexstring, NULL);

    // now create generate the thread data for each thread and run them
    for (t = 0; t < NT; ++t) {
        initThreadsData(t, &thread_data[t]);
        rc = pthread_create(&threads[t], NULL, searchString, (void *) &thread_data[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_attr_destroy(&attr);

    // here we wait for all the threads to complete before writing to the file 
    for (t = 0; t < NT; ++t) {
        rc = pthread_join(threads[t], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    // write to file and clean up     
    writeToFile();
    puts(out);
    releaseData();
    pthread_mutex_destroy(&mutexsv);
    pthread_mutex_destroy(&mutexstring); 
    pthread_exit(NULL);
    return 0;
}


void generateData(FILE *fptr, int *NT, int *NS) {
    // lets extract the meta data
    char _tempBuf[MAXLINE];
    if(fgets(_tempBuf, MAXLINE, fptr) == NULL) {
       // if we get in here something went wrong with the extraction
       puts("Error extracting data from file");
       exit(-1); 
    }
    *NT = atoi(_tempBuf); 
    if (fgets(_tempBuf, MAXLINE, fptr) == NULL) {
        puts("Error extracting data from file");
        exit(-1);
    }
    *NS = atoi(_tempBuf);
    if (fgets(_tempBuf, MAXLINE, fptr) == NULL) {
        puts("Error extracting data from file");
        exit(-1);
    }

    // here we remove the annoying newline from the matching string
    strtok(_tempBuf, NEWLINE);
    searchStr = malloc(strlen(_tempBuf) + 1);
    strcpy(searchStr, _tempBuf);

    // now lets place the remianing contents in a array 
    char s[MAXLINE];
    data_size = 0;

    while (fgets(s, MAXLINE, fptr))
       data_size++;

    rewind(fptr);

    for (int i = 0; i < 3; ++i)
       fgets(s, MAXLINE, fptr);

    dataArray = (char **) malloc(data_size * sizeof(char *));

    for (int i = 0; i < data_size; ++i) {
        fgets(s, MAXLINE, fptr); 
        strtok(s, NEWLINE);
        dataArray[i] = (char *) malloc(strlen(s) + 1);
        strcpy(dataArray[i], s);
    }
}




void initThreadsData(int id, t_data *data) {
    data->t_id = id;
    strcpy(data->string_found, "no");
    data->slice_index = -1;
    data->string_index = -1;
}


void *searchString(void *thread_data) {
    // for Part B we need to continue in a loop until a condition is met
    t_data *data = (t_data *) thread_data;
    
    // get an initial slice from SV
    int current_slice;
    getSlice(&current_slice, data);

    // begin search
    while (current_slice < (NS + 1)) {
        for (int i = data->array_start; i < data->array_end; ++i) {
            if (i >= data_size)
                break;
            if (!strcmp(dataArray[i], searchStr)) {
                strcpy(data->string_found, "yes"); 
                data->slice_index = current_slice - 1;
                data->string_index = i;
                break;
            }
        }
        // get a new slice 
        getSlice(&current_slice, data);
    }
    char _temp[MAXLINE];
    pthread_mutex_lock (&mutexstring);
    sprintf(_temp, "thread%d,\tfound %s,\tslice %d,\tposition %d\n" 
                                                , data->t_id
                                                , data->string_found
                                                , data->slice_index
                                                , data->string_index);
    strcat(out, _temp);
    pthread_mutex_unlock(&mutexstring); 
    pthread_exit(NULL);
}


void writeToFile() {
    FILE *fptr;
    fptr = fopen("OutB.txt", "w");
    fputs(out, fptr);
    fclose(fptr); 
}

void releaseData() {
    for (int i = 0; i < data_size; ++i) {
        free(dataArray[i]);
    }
    free(dataArray);
    free(searchStr);
}


void getSlice(int *current_slice, t_data *data) {
    pthread_mutex_lock (&mutexsv);
    *current_slice = SV++;

    // update the thread data
    data->array_start = (*current_slice - 1) * slice_size;
    data->array_end = data->array_start + slice_size;
    pthread_mutex_unlock (&mutexsv);
}
