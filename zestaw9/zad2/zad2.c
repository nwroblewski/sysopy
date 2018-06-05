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
#include <semaphore.h>

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
int inside = 0;
FILE *file_handle;


pthread_t *producers;
pthread_t *consumers;
sem_t buff_sem;
sem_t empty_buff_sem;
sem_t full_buff_sem;


void *producer(){
    size_t size = 0;
    char *line = NULL;
    while(1){
        sem_wait(&full_buff_sem);
        sem_wait(&buff_sem);
        
        if(getline(&line,&size,file_handle) <=0){
            sem_post(&buff_sem);
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
        sem_post(&empty_buff_sem);
        sem_post(&buff_sem);
    }
    return NULL;
}

void *client(){
    while(1){
        
        sem_wait(&empty_buff_sem);
        sem_wait(&buff_sem);

        char *line = malloc(sizeof(char) * 512);
        strcpy(line,buffer[consumer_pos]);


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
        if(buffer[consumer_pos]) free(buffer[consumer_pos]);        
        consumer_pos = (consumer_pos + 1) % N;
        inside --;
        sem_post(&full_buff_sem);
        sem_post(&buff_sem);
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
    printf("P: %d, K: %d, N: %d, filename: %s, L: %d ,search: %d, view: %d, nk: %d \n",P,K,N,filename,L,search_mode,view_mode,nk);
    fclose(file);
}
void clean_up(){
    if(consumers);
    for(int i = 0; i< K; i++){
        pthread_cancel(consumers[i]);
    }
    if(file_handle) fclose(file_handle);
    if(buffer) free(buffer);
    sem_destroy(&buff_sem);
    sem_destroy(&empty_buff_sem);
    sem_destroy(&full_buff_sem);
}

void sighandler(int signum){
    printf("Caught signal. Aborting. \n");
    exit(0);
}



int main(int argc, char **argv){
    
    atexit(*clean_up);
    signal(SIGINT,sighandler);
    signal(SIGALRM,sighandler);
    
    if(argc < 2){
        printf("GIVE ME PROPER ARGUMENTS NUMBER! <config_file> \n");
        exit(EXIT_FAILURE);
    }
    
    read_config(argv[1]);
    sem_init(&buff_sem,0,1);
    sem_init(&full_buff_sem,0,N);
    sem_init(&empty_buff_sem,0,0);

    if(nk > 0) alarm(nk);
    
    file_handle = fopen(filename,"r");
    producers = malloc(sizeof(pthread_t) * P);
    consumers = malloc(sizeof(pthread_t) * K);
    buffer = malloc(sizeof(char*) * N);
    for(int i = 0; i < N; i++){
        buffer[i] = NULL;
    }

    for(int i = 0; i < P; i++) pthread_create(&producers[i],NULL,&producer,NULL);
    
    for(int i = 0; i < K; i++) pthread_create(&consumers[i],NULL,&client,NULL);
    
    for(int i = 0; i < P; i++) pthread_join(producers[i], NULL);
    
    while(1){
        sem_wait(&buff_sem);
        if(inside == 0) break;
        sem_post(&buff_sem);
    }
    while(nk);
    return 0;
}