#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <zconf.h>

int stopped = 0;

void SIGINT_handle(int signum){
    printf("\nI've received signal that want me to terminate this program. So I will do this.\n");
    exit(0);
}

void SIGTSTP_handle(int signum){
    if(!stopped) printf("\nStopped here, waiting for CTRL+Z or CTRL+C \n");
    stopped = stopped == 0 ? 1 : 0;
}

void printTime(){
        time_t rawtime;
        struct tm* timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        printf("\n %s \n",asctime(timeinfo));
}


int main(int argc, char ** argv){
    struct sigaction action;
    action.sa_handler = SIGTSTP_handle; 
    sigemptyset(&action.sa_mask); //all signals excluded from the sa_mask
    action.sa_flags = 0;
    sigaction(SIGTSTP,&action,NULL); //oldact = NULL, 
    
    signal(SIGINT,SIGINT_handle); //przechwyt siginta do dyspozycji sigint_handle
    while(1){
        if(stopped != 1) printTime();
        sleep(1);
    }
    return 0;
}