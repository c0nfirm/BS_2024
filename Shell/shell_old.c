#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "vec.h"	

int argc = 0;
pid_t *proc;

/*stop shell id 0; stop wait id 1 with setting it to 0*/
int stop_wait = 0;

/*signal handler for strg + c*/
void sig_handler(int sig){
	stop_wait = 0;
	printf("wait terminated  --- TEST\n");
}

/*WAIT - Commmand
returns 1 to continue execution*/
/*int shell_wait(int args, pid_t *pid){
		int p_count = sizeof(pid), state, fin_count = 1;
		stop_wait = 1;

		printf("test - 1");

		while(stop_wait == 1){

			printf("test - 2");

			pid_t cpid = waitpid(-1, &state, 0);

			for(int i = 0; i < p_count; i++){
				printf("test - 3 : %d", p_count);
				if(args == (int)cpid){
					printf("test - 4");
					if(WIFEXITED(state) != 0){printf("[%d] TERMINATED\n", cpid);}
					else if(WIFSIGNALED(state) != 0){printf("[%d] SIGNALED\n", cpid);}
					else if(WSTOPSIG(state) != 0){printf("[%d] STOPPING\n", cpid);}
					printf("[%d] EXIT STATUS: %d\n", cpid, WEXITSTATUS(state));
					fin_count++;
				}
			}
			if (fin_count==p_count-1){return 1;}
		}
	
	return 1;
}*/

int shell_wait(char **args){
	int state;
	pid_t pid;

	proc = realloc(proc, sizeof(pid_t)*argc);

	if(proc == NULL){
		free(proc);
		fprintf(stderr, "Wait: Failed to Allocate Memory.");
		exit(EXIT_FAILURE);
	}

	stop_wait = 1;

	for(int i = 1; i < argc; i++){
		pid = (pid_t)atoi(args[i]);
		proc[i] = waitpid(pid, &state, WUNTRACED);
		if(proc[i] > 0 && stop_wait == 1){
			printf("[%d] Terminated\n", proc[i]);
			printf("[%d] Exit Status: %d\n", proc[i], state);
		}
	}

	free(proc);
	stop_wait = 0;

	return 1;
}

void wait_loop(){
	int i;
	pid_t pid[5];
	char **args;

	for(i=0; i<5; i++){
		if((pid[i] = fork()) == 0){
			sleep(1);
			exit(100 + i);
		}
	}
	
	for(i=0; i<5; i++){
		printf("Child %d\n", pid[i]);	
	}

	
	
	shell_wait(args);

	
};

int main() {
	/*Shell Loop - Command*/
	signal(SIGINT, sig_handler);

	wait_loop();
	return EXIT_SUCCESS;


}
