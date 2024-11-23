//PhilMods

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>

#define DELIM " \t\n"

char *entry;
char *currentDirectory;
int argc = 0;
int pipeIndex = 0;
int stop_wait = 0;
int cd_mode = 0;
pid_t *proc;
char* input = NULL;
char **beforePipe;
char **afterPipe;
int charSize = sizeof(char*);

/*signal handler for strg + c*/
void sig_handler(int sig){
	stop_wait = 0;
}


// get current working directory
void get_cwd(char loc[]) {

	char cur[PATH_MAX];
	if (getcwd(cur, sizeof(cur)) == NULL) {
		perror("getcwd() error");
		return;
	}

	if (strcmp(loc, "current") == 0) {
		currentDirectory = cur;
	}
	if (strcmp(loc, "entry") == 0) {
		entry = cur;
	}
	return;
}

char *relative_path() {

	if(strlen(currentDirectory) > strlen(entry)) { //lower than entry point
		int dif = strlen(currentDirectory) - strlen(entry);
		char *rel = malloc(sizeof(char) * dif);
		for(int i = 0; i < dif; i++) {
			rel[i] = currentDirectory[i+strlen(entry)];
		}
		return rel;
	}
	else if(strlen(currentDirectory) < strlen(entry)) { // higher than entry point
		return currentDirectory;
	}
	else { return ""; } // entry point
}

int run_cd(char **args) {
	
	if(args[1] == NULL && cd_mode > 0) {
		if(chdir(entry) != 0) {
			printf("error changing directory: %d\n", errno); return -1;
		}
	}
	else if(chdir(args[1]) != 0) {
		return -1; 
	}	
	
	get_cwd("current");
	return 1;
}

int runWithPipe(char **args) {
	printf("\ntest 1\n");

	int fileDesc[2]; //file descriptor: fd[0] for the read end of pipe, fd[1] for the write end of pipe.
	int status; //status info, used later by waidpid()
	pid_t processId;
	int pipeOK = pipe(fileDesc); //pipe system call, parameters fd[0], fd[1]. return 0 success / -1 on error
	if(pipeOK == -1) { 
		fprintf(stderr, "runWithPipe():: Pipe syscall failed. Command will not be executed"); 
		return -1;
	} 	
	//clear beforePipe and afterPipe memory from previous runs
	free(beforePipe);
	free(afterPipe);
	//fill with chars sequence for comman and arguments before (beforePipe) and after the pipe (afterPipe)
	beforePipe = malloc(charSize*(pipeIndex)+1); 
	afterPipe = malloc(charSize*(argc - pipeIndex));
	
	//if beforePipe or afterPipe is null, don't execute but return and print error					
	if(beforePipe == NULL || afterPipe == NULL) { 
		fprintf(stderr, "runWithPipe():: Failed to allocate memory for pipe. Command will not be executed");
		return(-1);
	}
	//fill beforepipe with String chars before pipe, and add \0 (null char) at the end 
	beforePipe[pipeIndex] = '\0'; 
	for(int argIndex = 0; argIndex < pipeIndex; argIndex++) {
		beforePipe[argIndex] = args[argIndex];
	}
	//fill afterPipe with String chars after the pipe 			
	int afterIndex = 0;
	for(int argIndex = pipeIndex+1; argIndex < argc; argIndex++) {
		afterPipe[afterIndex] = args[argIndex];
		afterIndex++;
	}
	//reset global pipeIndex for next pipe run			
	pipeIndex = 0;	
	
	//fork the child, set pipes [child -> parent], execvp beforePipe on child, afterPipe on parent
	processId = fork();
	if(processId < 0) {
		fprintf(stderr, "runWithPipe():: Could not fork. Command will not be executed."); 
		return -1;
	}
	else if (processId == 0) { 
		//child: handle command and args before the pipe
		dup2(fileDesc[1],1);    //write end of pipe, STDOUT_FILENO=1 (POSIX)
		close(fileDesc[0]);     //don't need the read end of pipe
		//execute beforePipe command and args
		execvp(beforePipe[0], beforePipe);
	}
	else { 
		//parent: handle command and args behind the pipe
		dup2(fileDesc[0],0);   //read end of pipe, STDIN_FILENO=0 (POSIX)
		close(fileDesc[1]);    //don't need the write end of pipe
		// wait for child complete beforePipe, since its output is needed as input for parent
		waitpid(processId, &status, WUNTRACED); //WUNTRACED allows parent to be returned from waitpid if child stopped/killed
		//execute afterPipe command and args
		execvp(afterPipe[0], afterPipe);
	}

	return -1;
}

int run(char **args) {
	int status;
	pid_t pid = fork();
	
	if(pid < 0) { fprintf(stderr, "run():: Failed to fork, command will not be executed. \n"); return -1; }
	
	if(pid == 0) { //child process
		printf("\ntest 2\n");
		if(pipeIndex > 0) { //if valid pipe-operator was found this is > 0
			runWithPipe(args);
		} else {
			printf("\ntest 3\n");
			execvp(args[0], args);
			exit(-1);
		}
	}
	else  { //parent process
		printf("\ntest 0.1\n");
		if(strcmp(args[argc-1], "&") == 0) { //background process
			printf("\ntest 0.2\n");
			printf("[%d]\n", pid);
			printf("\ntest 0.4\n"); 
		} 
		else { 
			printf("\ntest 0.3\n");
			waitpid(pid, &status, WUNTRACED); //not a background process => wait for completion
		} 
	} 
		
	return 1;
}

void exitShell() {
	printf("BYE!\n");
	//cleanup:
	//what else do we need to free?
	free(beforePipe);
	free(afterPipe);
	//free(currentDirectory); //not needed
	//free(entry);  //not needed
	exit(0);
}

void showHelp() {
	//we need some ASCII art here!
	puts("\n*** BS1S - Basic Simple 1NCr3D1813 Shell***"
		"\nList of available commands:"
		"\ncd"
		"\nwait (ToDo)"
		"\nhelp"
		"\nexit"
		"\nany general commands from Linux shell"
		"\ncan use simple pipe (max one pipe)");
	return;
}

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

	/*free(proc);*/
	stop_wait = 0;

	return 1;
}

int run_command(char **args) {
	if(argc == 0 || args[0] == NULL) { return -1; } 
	
	if(strcmp(args[0], "cd") == 0) {
		return run_cd(args);
	}
	else if(strcmp(args[0], "exit") == 0) {
		exitShell();
	}
	else if(strcmp(args[0], "help") == 0) {
		showHelp();
	}
	else if(strcmp(args[0], "wait") == 0) {
		shell_wait(args);
		return 1;
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
		if(strcmp(cur, "|") == 0) { pipeIndex = pos; }	//for easier splitting of the arguments later
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
	args[pos] = NULL; //to detect last element, e.g. used in run_cd() to check for args
	return args;
}

char *readinput() {
	char *line = malloc(100);
	char *linep = line;
    size_t lenmax = 100;
	size_t len = lenmax;
    int c;
    
    if(line == NULL)
        return NULL;

    while(1) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

void shell_loop() {
	char **args;
	
	while(1) {
		pipeIndex = 0;	//reset pipeIndex
		printf("%s/> ", relative_path());
		args = split(readinput());
		run_command(args);
		free(args);
	}	
}

int main(void) {
	signal(SIGINT, sig_handler);

	get_cwd("entry"); //get entry point
	get_cwd("current"); //get current path
	proc = NULL;
	
	//welcome message
	printf("WELCOME TO BS1 SHELL\n");	
	shell_loop(); //run shell in a loop	
	return 0;
}
