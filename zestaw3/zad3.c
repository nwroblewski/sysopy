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
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>

int limits(char * time_limit, char * memo){
    unsigned long long int time_int = strtol(time_limit,NULL,10);
    struct rlimit cpu_limit;
    cpu_limit.rlim_max =(rlim_t) time_int;
    cpu_limit.rlim_cur= (rlim_t) time_int; 
    
    if(setrlimit(RLIMIT_CPU, &cpu_limit) != 0){
        printf("SOMETHING FAILED WHEN SETTING CPU USAGE TIME LIMIT!!!11!! \n");
        return -1;
    }

    unsigned long long int memory = strtol(memo,NULL,10);
    struct rlimit memo_limit;
    memo_limit.rlim_max =  (rlim_t) memory * 1024 * 1024; //converted from bytes to megabytes
    memo_limit.rlim_cur =  (rlim_t) memory * 1024 * 1024;

    if(setrlimit(RLIMIT_DATA,&memo_limit) != 0){
        printf("SOMETHING FAILED WHEN SETTING MEMORY LIMIT!1111111!!!! \n");
        return -1;
    }
    return 1;
}


int batch_interprete(char * path,char *time, char* memo){
    FILE* file;
    file = fopen(path,"r"); 
    if(!file){
        printf("Couldn't open a file! \n");
        return -1;
    }
    int arg_no;
    ssize_t checker;
    size_t buffer_size = 256;
    char * params[48];
    char * lineptr = malloc(sizeof(char) * buffer_size);
    int status;
    struct rusage usage_0;
    getrusage(RUSAGE_CHILDREN,&usage_0);
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
            limits(time,memo);  // nałożenie limitów na proces
            execvp(params[0],params);
            exit(1);
        }
        wait(&status);
        if(status){
            printf("\n %s failed! \n",params[0]);
            exit(1);
        }
        struct rusage usage_1;
        getrusage(RUSAGE_CHILDREN, &usage_1);
        struct timeval usec;            //timeval ma sekundy i mikrosekundy (z manuala)
        struct timeval ssec;
        timersub(&usage_1.ru_utime,&usage_0.ru_utime,&usec); //timersub(a,b,c) -> c = b - a
        timersub(&usage_1.ru_stime,&usage_0.ru_stime,&ssec);
        usage_0 = usage_1;

        printf("%s \n", params[0]);
        printf("USER TIME: %d.%d  and  SYSTEM TIME: %d.%d  MEMO USED: %d \n",usec.tv_sec,usec.tv_usec,ssec.tv_sec,ssec.tv_usec,usage_1.ru_maxrss);


    }
    if(checker < 0 ){
        perror("There was an error while reading from file! \n");
        return(-1);
    }

    fclose(file);
    return 0;
    
}

int main(int argc, char** argv){
    if(argc < 4){
        printf("TOO FEW ARGUMENTS PROVIDED! \n");
        return 1;
    }
    if(batch_interprete(argv[1],argv[2],argv[3]) < 0 ){
        perror("Something went wrong with interpreter! :/ \n");
        return 1;
    }
    return 0;
}