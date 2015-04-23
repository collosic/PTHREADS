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
    
    int NT;
    int NS;
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
    int extra = !(data_size % NS) ? 0 : NS - (data_size % NS);
    int slice_size = (data_size + extra) / NS;
    int slice_start = 0;
    int slice_end = slice_size;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexstring, NULL);

    // now create generate the thread data for each thread and run them
    for (t = 0; t < NT; ++t) {
        initThreadsData(t, slice_start, slice_end, &thread_data[t]);
        slice_start = slice_end;
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
    }
     
    writeToFile();
    releaseData();
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

    // lets now rewind the file back to the beginning
    rewind(fptr);

    // skip the first three lines
    for (int i = 0; i < 3; ++i)
        fgets(s, MAXLINE, fptr);

    // now lets allocate our array
    dataArray = (char **) malloc(data_size * sizeof(char *));

    for (int i = 0; i < data_size; ++i) {
        fgets(s, MAXLINE, fptr); 
        strtok(s, NEWLINE);
        dataArray[i] = (char *) malloc(strlen(s) + 1);
        strcpy(dataArray[i], s);
    }
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
        if (data->array_end > data_size)
            break;
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

void releaseData() {
    for (int i = 0; i < data_size; ++i) {
        free(dataArray[i]);
    }
    free(dataArray);
    free(searchStr);
}
