#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int P;
int K;
int N;
char *filename;
int L;  
int search_mode;  // 0 - producer doesn't print info about reading from file, 1 - does
int view_mode;  // 0 - equal L , 1 - greater than L, 2 - less than L
int nk;
char **buffer;
int producer_pos = 0; // first ready-to-write position in buffer
int consumer_pos = 0; // first ready-to-read position in buffer
int inside = 0; // number of lines elements currently in buffer
FILE *file_handle;
pthread_t *producers;
pthread_t *consumers;
/* MUTEXES */

pthread_mutex_t buff_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buff_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t buff_full = PTHREAD_COND_INITIALIZER;


/* Threads routines, for producer and client */

void *client(){
    char *line;
    while(1){
        pthread_mutex_lock(&buff_mutex);
        while(inside <= 0){
            pthread_cond_wait(&buff_empty,&buff_mutex);
        }
    
        line = buffer[consumer_pos];
        buffer[consumer_pos] = NULL;

        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        if(line[0] != '\0' && line != NULL){
            int check;
            switch(view_mode){
                case 0: check = (strlen(line) == L); 
                break;

                case 1: check = (strlen(line) > L);
                break;

                case 2: check = (strlen(line) < L);
                break;
            }
            if(check) printf("Index: %i, content: %s \n",consumer_pos,line);
        }
        if(line) free(line);
        consumer_pos = (consumer_pos + 1) % N;
        inside --;
        if(inside == N - 1) pthread_cond_broadcast(&buff_full);
        pthread_mutex_unlock(&buff_mutex);
    }
    return NULL;
}


void *producer(){
    size_t size = 0;
    char *line = NULL;
    while(1){
        pthread_mutex_lock(&buff_mutex);
        while(inside >= N){
            pthread_cond_wait(&buff_full,&buff_mutex);
        }

        if(getline(&line,&size,file_handle) <=0){
            pthread_mutex_unlock(&buff_mutex);
            printf("Stopped reading a file. \n");
            break;
        }
        buffer[producer_pos] = malloc(sizeof(char)*size);
        strcpy(buffer[producer_pos],line);
        if(line){
            free(line);
            line = NULL;
        }        
        
        //If printing mode is needed
        if(search_mode) printf("Producer puts line at %i. It's %i line in the buffer.\n",producer_pos,inside);
        
        producer_pos = (producer_pos + 1) % N;
        inside ++;
        if(inside == 1){
            pthread_cond_broadcast(&buff_empty);
        }
        pthread_mutex_unlock(&buff_mutex);
    }
    return NULL;
}


void read_config(char *path){
    filename = malloc(sizeof(char)*100);
    FILE* file = fopen(path,"r+");
    fscanf(file,"%i",&P);
    fscanf(file,"%i",&K);
    fscanf(file,"%i",&N);
    fscanf(file,"%s",filename);
    fscanf(file,"%i",&L);
    fscanf(file,"%i",&search_mode);
    fscanf(file,"%i",&view_mode);
    fscanf(file,"%i",&nk);
    fclose(file);
}

void manage_threads(){

    for(int i = 0; i < P; i++){
        pthread_create(&producers[i],NULL,&producer,NULL);
    }
    
    for(int i = 0; i < K; i++){
        pthread_create(&consumers[i],NULL,&client,NULL);
    }


    for(int i = 0; i < P; i++){
        pthread_join(producers[i],NULL);
    }

    while (1){
        pthread_mutex_lock(&buff_mutex);
        if (inside == 0) break;
        pthread_mutex_unlock(&buff_mutex);
    }
}

void clean_up(){
    printf("Cleaning \n");
    if(consumers);
    for(int i = 0; i< K; i++){
        pthread_cancel(consumers[i]);
    }
    if(file_handle) fclose(file_handle);
    if(buffer) free(buffer);
    pthread_mutex_destroy(&buff_mutex);
    pthread_cond_destroy(&buff_empty);
    pthread_cond_destroy(&buff_full);
}

void sighandler(int signum){
    printf("Caught signal. Aborting. \n");
    exit(0);
}



int main(int argc, char **argv){
    
    atexit(*clean_up);
    signal(SIGINT,sighandler);
    signal(SIGALRM,sighandler);
    if(argc != 2){
        printf("Provide me with proper arguments! <config_filename>\n");
        exit(EXIT_FAILURE);
    }
    read_config(argv[1]);
    if(nk > 0){
        alarm(nk);
    }
    buffer = malloc(sizeof(char*)*N);
    producers = malloc(sizeof(pthread_t) * P);
    consumers = malloc(sizeof(pthread_t) * K); 
    for(int i = 0; i < N; i++){
        buffer[i] = NULL;
    }
    file_handle = fopen(filename,"r");
    manage_threads();
    while(nk);
    return 0;
}