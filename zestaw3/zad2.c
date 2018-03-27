/*  
    Created by Mikolaj Wroblewski
    AGH UST Computer Science
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <zconf.h>
#include <sys/times.h>
#include <sys/wait.h>


int batch_interprete(char * path){
    FILE* file;
    file = fopen(path,"r"); 
    if(!file){
        printf("Couldn't open a file! \n");
        return 1;
    }
    int arg_no;
    ssize_t checker;
    size_t buffer_size = 256;
    char * params[48];
    char * lineptr = malloc(sizeof(char) * buffer_size);
   // char *const to_exec = malloc(sizeof(char) * buffer);s
    int status;
    fseek(file,0,0);
    while((checker = getline(&lineptr,&buffer_size,file) > 0 )){    
        arg_no = 0;
        while((params[arg_no] = strtok(arg_no == 0 ? lineptr : NULL, " \n\t")) != NULL){
            arg_no++;
            if(arg_no >= 48){
                printf("You provided too many arguments! \n");
                return -1;
            }
        }
        pid_t proc;
        if((proc = fork()) == 0 ){
            execvp(params[0],params);
        }

        wait(&status);
        if(status){
            printf("%s failed!",params[0]);
            exit(1);
        
        }
    }
    if(checker < 0 ){
        perror("There was an error while reading from file!");
        return(-1);
    }

    fclose(file);
    return 1;
    
}


int main(int argc, char** argv){
    if(argc < 2){
        printf("TOO FEW ARGUMENTS PROVIDED! \n");
        return 1;
    }
    if(batch_interprete(argv[1]) < 0 ){
        perror("ten getline to chyba jednak nie działa :/");
        exit(1);
    }
    return 1;
}