/* Christian Collosi
 * CS131 - Lab1 - Part B
 *
 *
 */

// include files here
#include "pthreads.h"

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
    slice_size = DATA_SIZE / NS;

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
    
    for (int i = 0; i < DATA_SIZE; ++i) {
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
            if (!strcmp(dataArray[i], searchStr)) {
                strcpy(data->string_found, "yes"); 
                data->slice_index = current_slice;
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
    fptr = fopen("out.txt", "w");
    fputs(out, fptr);
    fclose(fptr); 
}

void releaseData() {
    for (int i = 0; i < DATA_SIZE; ++i) {
        free(dataArray[i]);
    }
    free(searchStr);
}


void getSlice(int *current_slice, t_data *data) {
    pthread_mutex_lock (&mutexsv);
    *current_slice = SV++;

    // update the thread data
    data->array_start = (*current_slice - 1) * slice_size;
    data->array_end = data->array_start + (slice_size - 1);
    pthread_mutex_unlock (&mutexsv);
}
