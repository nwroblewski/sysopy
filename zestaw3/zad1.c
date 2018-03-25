//
// Created by mikolaj on 16.03.18.
//

#define __USE_XOPEN
#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <ftw.h>
#include <math.h>
#include <sys/types.h>


int const size = 4096;

char buff[4096];

int long_int_length(long long int size){
    int length = 0;
    while(size!=0){
        length++;
        size /= 10;
    }
    return length;
}

int compare_dates(char * user_date,char * operant,time_t file_date){
    struct tm* time = malloc(sizeof(struct tm));
    struct tm* time2 = malloc(sizeof(struct tm));

    char buff[20];
    strftime(buff,20,"%Y-%m-%d %H:%M:%S",localtime(&file_date));

    //first i need to convert user provided date to "normal" format
    strptime(user_date,"%Y-%m-%d %H:%M:%S",time);
    strptime(buff,"%Y-%m-%d %H:%M:%S",time2);
    time_t _time2 = mktime(time2);
    time_t _time = mktime(time);

    if(operant[0] == '='){
        return fabs(difftime(_time,_time2)) > 0.001 ? 0 : 1;
    }
    else if(operant[0] == '>'){
        return difftime(_time,_time2) > 0 ? 0 : 1;
    }
    else if(operant[0] == '<'){
        return difftime(_time,_time2) < 0 ? 0 : 1;
    }
    else {

        printf("U PROVIDED ME WITH WRONG OPERAND!!11!");
        exit(1);
    }
}

void print_info(char * path,struct stat file_stat){
    char buff[20];
    strftime(buff,20,"%Y-%m-%d %H:%M:%S",localtime(&file_stat.st_mtim.tv_sec));
    printf("%s\t" ,path);
    printf("\t\t\t");
    printf("%lld",(long long int)file_stat.st_size);
    printf("\t\t\t");
    printf( (S_ISDIR(file_stat.st_mode)) ? "d" : "-");
    printf( (file_stat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (file_stat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (file_stat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (file_stat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (file_stat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (file_stat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (file_stat.st_mode & S_IROTH) ? "r" : "-");
    printf( (file_stat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (file_stat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\t\t");
    printf("%s ",buff);
    printf("\n");
}


//prints files inside directory, ignores hidden files
void traverse_dir(char* path, char* date, char* operant){

    DIR * dir = opendir(path);
    if(dir == NULL){
        perror("Error with opening directory");
        exit(1);
    }
    char path_holder[size];

    struct stat stats;
    struct dirent *current;
    current = readdir(dir);
    while(current!=NULL){

        strcpy(path_holder, path);
        strcat(path_holder, "/");
        strcat(path_holder, current->d_name);
        stat(path_holder,&stats);

        if (current->d_name[0] == '.') {
            current = readdir(dir);
            continue;
        }

        if (current->d_type == DT_DIR){
            printf("%s \n",current->d_name);
            pid_t proc;
            
            if(proc = fork() == 0){
                traverse_dir(path_holder,date,operant);
                exit(0);
            }
        }
        if (current->d_type == DT_REG && compare_dates(date,operant,stats.st_mtim.tv_sec)){
            print_info(path_holder,stats);
        }
        else{
            current = readdir(dir);
            continue;
        }
        current = readdir(dir);
    }
    closedir(dir);
}


int main(int argc, char ** argv){
    if(argc < 4){
        printf("You provided too few arguments! \n");
        exit(1);
    }
    char * path = argv[1];

    char * operand = argv[3];

    char * date = argv[2];

    traverse_dir(path,date,operand);
}