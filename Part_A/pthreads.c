/*
 *
 *
 *
 *
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
    
    // lets extract the meta data
    char *results = NULL;
    char _tempBuf[MAXLINE];
    results = fgets(_tempBuf, MAXLINE, fptr);
    const int NT = atoi(_tempBuf); 
    results = fgets(_tempBuf, MAXLINE, fptr);
    const int NS = atoi(_tempBuf);
    results = fgets(_tempBuf, MAXLINE, fptr);  
    searchStr = _tempBuf;
    strtok(searchStr, NEWLINE);

    // now lets place the remianing contents in a array 
    char s[MAXLINE];
    
    for (int i = 0; i < DATA_SIZE; ++i) {
        fgets(s, MAXLINE, fptr); 
        strtok(s, NEWLINE);
        dataArray[i] = (char *) malloc(strlen(s));
        strcpy(dataArray[i], s);
    }
    // close the file
    fclose(fptr);

    // construct pthreads
    t_data thread_data[NT];
    pthread_t threads[NT]; 
    pthread_attr_t attr;
    void *status;
    int rc;
    long t;
    int slice_size = DATA_SIZE / NS;
    int slice_start = 0;
    int slice_end = slice_size - 1;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexstring, NULL);

    // now create generate the thread data for each thread and run them
    for (t = 0; t < NT; ++t) {
        initThreadsData(t, slice_start, slice_end, &thread_data[t]);
        slice_start = slice_end + 1;
        slice_end += slice_size;
        rc = pthread_create(&threads[t], NULL, searchString, (void *) &thread_data[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }


    // pass the thread data over to 
    pthread_attr_destroy(&attr);

    // here we wait for all the threads to complete before writing to the file 
    for (t = 0; t < NT; ++t) {
        rc = pthread_join(threads[t], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        // set out the output file
    }
     
    writeToFile();
    pthread_mutex_destroy(&mutexstring); 
    pthread_exit(NULL);
    return 0;
}


void initThreadsData(int id, int start, int end, t_data *data) {
    data->t_id = id;
    strcpy(data->string_found, "no");
    data->slice_index = -1;
    data->string_index = -1;
    data->array_start = start;
    data->array_end = end;
}

void *searchString(void *thread_data) {
    t_data *data = (t_data *) thread_data;
    for (int i = data->array_start; i < data->array_end; ++i) {
        if (!strcmp(dataArray[i], searchStr)) {
            strcpy(data->string_found, "yes"); 
            data->slice_index = data->t_id;
            data->string_index = i;
            break;
        }
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
