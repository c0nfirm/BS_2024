#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#include "array.h"

#define DELIM " \t\n"

char *entry;
char *cwd;
int argc = 0;
int pipe_index = 0;
int wait_check = 0;
int cd_mode = 0;
pid_t *processes;

static char* builtin_commands[] = {"cd", "cd-mode", "wait", "exit", "lhelp"};
int bi_count = 5;


void fetch_entrydir() {
	int size = 4096; //PATH_MAX wouldn't work;
	char *cur = malloc(sizeof(char)*size);
	
	if(cur == NULL) { fprintf(stderr, "Failed to allocate memory, exiting. "); return; }
	
	if (getcwd(cur, sizeof(char)*size) == NULL) {
		printf("fetch_cwd()::Failed to fetch current working directory. Directory has not been changed. \n");
		return;
	}
	
	entry = cur;
	return;
}

//yes it's the same method twice...
void fetch_cwd() {
	int size = 4096; //PATH_MAX wouldn't work;
	char *cur = malloc(sizeof(char)*size);
	
	if(cur == NULL) { fprintf(stderr, "Failed to allocate memory, exiting. "); exit(18); }
	
	if (getcwd(cur, sizeof(char)*size) == NULL) {
		printf("fetch_cwd()::Failed to fetch current working directory. Directory has not been changed. \n");
		return;
	}
	
	cwd = cur;
	return;
}

char *relative_path() {

	if(strlen(cwd) > strlen(entry)) { // can display as ./.../
		int dif = strlen(cwd) - strlen(entry);
		char *rel = malloc(sizeof(char) * dif+1);
		for(int i = 0; i < dif; i++) { //build display for relative path
			rel[i] = cwd[i+strlen(entry)];
		}
		rel[dif] = '\0';
		//printf("relative string is %s", rel);
		return rel;
	}
	else if(strlen(cwd) < strlen(entry)) { //cannot display as relative, display as absolute
		return cwd;
	}
	else { return "."; } //if(strlen(cwd) == strlen(entry))
}

//deprecated. didn't work, no time to fix
char *pipe_split(char *str, int index) {
	return "";
}

int b_help(char **args) { 	//help skeleton

	if(argc <= 1) {
		printf("The following commands are available:\n\n");
		for(int i = 0; i < bi_count; i++) {
			printf("\t%d. %s\n", i, builtin_commands[i]);
		}
		
		printf("\nType \"lhelp [command]\" for details.\n");
	}
	else {
		if(strcmp(args[1], "cd") == 0) {
			printf("\tcd - change directory:\n\tChanges the current working directory to (relative) directory specified in args[1].");
			printf("\n\tIf no arguments are passed and set_cd > 0, cd will change to original entry point of the shell.\n");
			printf("\n\tCall cd-mode [value] to set set_cd value. set_cd = max{0,value}\n\n");
		}
		else if(strcmp(args[1], "wait") == 0) {
			printf("placeholder help wait\n");
		}
		else if(strcmp(args[1], "exit") == 0) {
			printf("placeholder help cd\n");
		}
		else {
			printf("no help available for \"%s\"\n", args[1]);
		}
	}
	
	return 1;
}

int b_cdmode(char **args) { //option for cd behaviour
	if(args[1] == NULL) { printf("\tUsage: cd-mode \"value\"\n"); return 1; }
	
	int value = 0;
	if (args[1] > 0) {value = atoi(args[1]);} //atoi is not safe
	cd_mode = value;
	
	return 1;
}

int b_cd(char **args) {
	
	//user can choose between 2 cd modes. 0 = normal cd, 1 = cd without arguments returns to entry point of shell
	if(args[1] == NULL && cd_mode > 0) {
		if(chdir(entry) != 0) {
			printf("error changing directory: %d\n", errno); return -1;
		}
		//printf("reverted directory to entry point: %s\n", entry);
	}
	
	else if(chdir(args[1]) != 0) {
		return -1; //couldn't find/moveto change_directory
	}	
	
	fetch_cwd();
	return 1;
}

int b_wait(char **args) { //cannot cancel the wait, we have no idea how to without killing the child.
	int status;	
	pid_t pid;
	processes = realloc(processes, sizeof(pid_t)*argc);
	
	if(processes == NULL) {
		free(processes);
		fprintf(stderr, "b_wait():: Failed to allocate memory.Exitiing."); return 200; //for exiting. this is not good.
	}
	
	wait_check = 1;
	
	//runs through all processes specified as args in the wait call, then just waits for the first one. 
	//once thats completed it waits for the next, and so on. 
	//any childs completed "out of order" are caught in handle_SIGCHLD and processed like here
	//this is only necessary to make sure that status messages are sent in order of completion AND on time,
	//not in the order they appear in the call, (which is also delayed by processes longer than their successors)
	for(int i = 1; i < argc; i++) { //loop through processes, compare id
		pid = (pid_t)atoi(args[i]);
		//printf("STARTING WAIT FOR PROCESS: [%d]\n", pid);
		processes[i] = waitpid(pid, &status, WUNTRACED);
		if(processes[i] > 0 && wait_check == 1) {printf("[%d] TERMINATED\n[%d] EXIT STATUS: %d\n", processes[i], processes[i], status);}
	}
	
	free(processes);
	wait_check = 0;
	
	return 1;
}

int run_piped(char **args) {
	int fd[2];
	int status;
	pid_t pid;
	if(pipe(fd) == -1) {fprintf(stderr, "run_piped():: Pipe failed. Command will not be executed"); return -1;};
	
	//following block splits the arguments into the one before and the one after the pipe
	char **first = malloc(sizeof(char*)*(pipe_index)+1);
	char **second = malloc(sizeof(char*)*(argc - pipe_index));
						
	if(first == NULL || second == NULL) { printf("run_piped()::Failed to allocate memory. Exiting"); exit(35); } 
	for(int i = 0; i < pipe_index; i++) {
		first[i] = args[i];
		printf("%s ", first[i]);
	}
	first[pipe_index] = '\0';
	printf("\n");
			
	int k = 0;
	for(int i = pipe_index+1; i < argc; i++) {
		second[k] = args[i];
		printf("%s ", second[k]);
		k++;
	}
	printf("\n");			
	pipe_index = 0;	
	
	//fork the child, set pipes [child -> parent], execvp first on child, second on parent
	if((pid = fork()) < 0) {
		fprintf(stderr, "run_piped()::Failed to fork. Command will not be executed."); return -1;
	}
	else if (pid == 0) { //child
		dup2(fd[1],1);
		close(fd[0]);
		
		execvp(first[0], first);
	}
	else { //parent
		dup2(fd[0],0);
		close(fd[1]);
		
		waitpid(pid, &status, WUNTRACED); //parent waits for the child to complete since it needs the output
		execvp(second[0], second);
	}
	
	free(first);
	free(second);
	
	return -1;
}

int run(char **args) {
	int status;
	pid_t pid = fork();
	
	if(pid < 0) { fprintf(stderr, "run():: Failed to fork, command will not be executed. \n"); return -1; }
	
	if(pid == 0) { //child
		if(pipe_index > 0) { //if valid pipe-operator was found this is > 0
			run_piped(args);
		} else {
			execvp(args[0], args);
			exit(-1);
		}
	}
	else  { //parent
		if(strcmp(args[argc-1], "&") == 0) { printf("[%d]\n", pid); } //background process
		else { 
			waitpid(pid, &status, WUNTRACED); //not a background process => wait for completion
		} 
	} 
		
	return 1;
}

int command(char **args) {
	
	if(argc == 0 || args[0] == NULL) { return -1; } 
	
	if(strcmp(args[0], "cd") == 0) {
		return b_cd(args);
	}
	else if(strcmp(args[0], "wait") == 0) {
		return b_wait(args);
	}
	else if(strcmp(args[0], "cd-mode") == 0) {
		return b_cdmode(args);
	}
	else if(strcmp(args[0], "exit") == 0) {
		return 200; //OK
	}
	else if(strcmp(args[0], "lhelp") == 0) {
		return b_help(args);
	}
	else {
		return run(args);
	}
	
	return -1;
}

char **split(char *line) {
	int bufsize = 64;
	char **args = malloc(sizeof(char*)*bufsize); //can hold bufsize number of args. hopefully 64 is enough 
	char *cur;
	if(args == NULL) { fprintf(stderr, "split()::Failed to allocate memory, exiting. \n"); exit(11); }
	
	int pos = 0;
	cur = strtok(line, DELIM); 	//split by delimiter
	while(cur != NULL) {
		//printf("%d, %s", pos, cur);
		args[pos] = cur;
		if(strcmp(cur, "|") == 0) { pipe_index = pos; }	//for easier splitting of the arguments later
		pos++;
		
		if(pos >= bufsize) {
			bufsize *= 2;
			char **temp = realloc(args, sizeof(char*)*bufsize);
			if(!temp) { fprintf(stderr, "split()::Failed to reallocate memory, exiting. \n"); exit(12); }
			args = temp;
		}
		
		cur = strtok(NULL, DELIM);
	}
	
	argc = pos;
	args[pos] = NULL; //to detect last element, e.g. used in b_cd() to check for args
	return args;
}

char *readinput() { //how to do this with getline()?
	
	size_t bufsize = 512; //if this is too small...
	char *buf = malloc(sizeof(char)*bufsize); //malloc(size * sizeof(char));
	if(!buf) { fprintf(stderr, "Failed to allocate memory. Exiting."); exit(10); }
	//if(getline(&buf, &n, stdin) == -1) { fprintf(stderr, "readinput()::Failed to read command. \n"); }
	//buf = strtok(buf, "\n");	
	int pos = 0;	
	int c = getchar();
	while(c != EOF && c != '\n') {
		buf[pos] = c;
		pos++;
		
		if(pos >= bufsize) {
			bufsize *= 2;
			char *temp = realloc(buf, sizeof(char)*bufsize);
			if(!temp) { fprintf(stderr, "readinput()::Failed to reallocate memory, exiting. \n"); exit(12); }
			buf = temp;
		}
		
		c = getchar();
		//printf("%d", pos);
	}
	buf[pos] = '\0';
	
	//printf("debug:: allocated %d characters: %s\n", pos, buf);
	return buf;
}

void loop() {
	char **args;
	int ret = 0;
	
	while(1) {
		pipe_index = 0;	//no time, quick fix. remove this and run a | command, then run invalid command (e.g. 000) to reproduce bug; pipe_index not getting reset correctly somewhere
		printf("%s/> ", relative_path());
		args = split(readinput());
		ret = command(args);
		
		free(args);
		
		if(ret == 200) {
			free(cwd);
			free(entry);
			exit(200);
		}
	}
	
}

//couldn't get breaking out of waitpid with CTRL-C to work without killing the children.
//Therefore doesn't work, but there will not be output when wait (aka the child) is cancelled with CTRL-C at least
void handle_SIGINTp(int sig) { //not supposed to quit shell with CTRL-C
	wait_check = 0; //so terminated waited children don't produce output
	return;
}

void handle_SIGCHLD(int sig) {
		
	//catches all child processes that exit before the waitpid() in bi_wait() returns, aka children that are faster than the first entry
	pid_t pid;
	int status;
		
	pid = waitpid(-1, &status, WNOHANG); //-1 should work?
	if(pid > 0 && wait_check == 1) {
		printf("[%d] TERMINAED\n[%d] EXIT STATUS: %d\n", pid, pid, status); //just report em all rather than random sets of it
		/*
		for(int i = 0; i < argc; i++) {
			
			//printf("[%d] == [%d] ?\n", pid, processes[i]);
			if(pid == processes[i]) {
				printf("[%d] TERMINAED\n[%d] EXIT STATUS: %d\n", pid, pid, status);
			}		
		}
		*/
	}
	
	return;
}

int main(void) {
	
	signal(SIGINT, handle_SIGINTp); //assign handler for SIGINT
	signal(SIGCHLD, handle_SIGCHLD); //assign handler for SIGCHLD
	
	fetch_entrydir(); //get cwd right at the start -> entry point
	fetch_cwd(); //for relative path
	processes = NULL;
	
	//welcome message
	printf("\tWelcome to shell. Please, type \"lhelp\" for available commands.\n");
	
	loop(); //start command loop
	
	return 0;
}