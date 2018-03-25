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

int launch_command(char * command){
    //defining a char * [] used by execvp command 
    char * prepared[] = {
        "/bin/sh",
        "-c",
        command,
        NULL
    };
    if(execvp(prepared[0],prepared) < 0){
        perror("Something went wrong when executing a command!");
        exit(1);
    }  
}

//processess a file with bash commands to be executed
int batch_interprete(char * path){
    FILE* batch_commands;
    batch_commands = fopen(path,"r"); 
    
    ssize_t checker;
    size_t buffer = 256;
    char * lineptr = malloc(sizeof(char) * buffer);
    char *const to_exec = malloc(sizeof(char) * buffer);
    int i = 1;
    int status;
    fseek(batch_commands,0,0);
    while((checker = getline(&lineptr,&buffer,batch_commands) > 0 )){    
        pid_t proc;
        
        if(proc = fork() == 0){
            launch_command(lineptr);
            exit(0);
        }
        wait(&status);
        if(status){
            exit(1);
        }
    }
    if(checker < 0 ){
        perror("There was an error while reading from file!");
        return(-1);
    }

    fclose(batch_commands);
    return 1;
    
}


int main(int argc, char** argv){

    if(batch_interprete("/home/mikolaj/SYSOPS/test.txt") < 0 ){
        perror("ten getline to chyba jednak nie działa :/");
        exit(1);
    }
    return 1;
}