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
char *cwd;
int argc = 0;
int pipe_index = 0;
int wait_check = 0;
int cd_mode = 0;
pid_t *processes;
char* input = NULL;

// get current working directory
void get_cwd(char loc[]) {

	char cur[PATH_MAX];
	if (getcwd(cur, sizeof(cur)) == NULL) {
		perror("getcwd() error");
		return;
	}

	if (strcmp(loc, "current") == 0) {
		cwd = cur;
	}
	if (strcmp(loc, "entry") == 0) {
		entry = cur;
	}
	return;
}

char *relative_path() {

	if(strlen(cwd) > strlen(entry)) { //lower than entry point
		int dif = strlen(cwd) - strlen(entry);
		char *rel = malloc(sizeof(char) * dif);
		for(int i = 0; i < dif; i++) {
			rel[i] = cwd[i+strlen(entry)];
		}
		return rel;
	}
	else if(strlen(cwd) < strlen(entry)) { // higher than entry point
		return cwd;
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

int run_piped(char **args) {
	int fileDesc[2]; //file descriptor: fd[0] for the read end of pipe, fd[1] for the write end of pipe.
	int status; //status info for waidpid
	pid_t processId;
	if(pipe(fileDesc) == -1) { //pipe system call, parameters fd[0], fd[1]. return 0 success / -1 on error
		fprintf(stderr, "run_piped():: Pipe failed. Command will not be executed"); 
		return -1;
	}; 	
	//following block splits the arguments into the one before and the one after the pipe
	char **beforePipe = malloc(sizeof(char*)*(pipe_index)+1); 
	char **afterPipe = malloc(sizeof(char*)*(argc - pipe_index));
	
	//if beforePipe or afterPipe is null					
	if(beforePipe == NULL || afterPipe == NULL) { 
		printf("run_piped()::Failed to allocate memory. Exiting"); 
		exit(35); // self-defined error code for memory alloc fail
	} 
	for(int argIndex = 0; argIndex < pipe_index; argIndex++) {
		beforePipe[argIndex] = args[argIndex];
		printf("%s ", beforePipe[argIndex]);
	}
	beforePipe[pipe_index] = '\0';
	printf("\n");
			
	int afterIndex = 0;
	for(int argIndex = pipe_index+1; argIndex < argc; argIndex++) {
		afterPipe[afterIndex] = args[argIndex];
		printf("%s ", afterPipe[afterIndex]);
		afterIndex++;
	}
	printf("\n");			
	pipe_index = 0;	
	
	//fork the child, set pipes [child -> parent], execvp beforePipe on child, afterPipe on parent
	if((processId = fork()) < 0) {
		fprintf(stderr, "run_piped()::Failed to fork. Command will not be executed."); return -1;
	}
	else if (processId == 0) { //child
		dup2(fileDesc[1],1);    //write end of pipe
		close(fileDesc[0]);
		//execute beforePipe
		execvp(beforePipe[0], beforePipe);
	}
	else { //parent
		dup2(fileDesc[0],0);   //read end of pipe
		close(fileDesc[1]);
		// wait for child complete beforePipe, since its output is needed as input for parent
		waitpid(processId, &status, WUNTRACED); //WUNTRACED allows parent to be returned from waitpid if child stopped/killed
		//execute afterPipe (
		execvp(afterPipe[0], afterPipe);
	}
	
	//deallocate memory used by beforePipe and afterPipe. 
	free(beforePipe);
	free(afterPipe);
	
	return -1;
}

int run(char **args) {
	int status;
	pid_t pid = fork();
	
	if(pid < 0) { fprintf(stderr, "run():: Failed to fork, command will not be executed. \n"); return -1; }
	
	if(pid == 0) { //child process
		if(pipe_index > 0) { //if valid pipe-operator was found this is > 0
			run_piped(args);
		} else {
			execvp(args[0], args);
			exit(-1);
		}
	}
	else  { //parent process
		if(strcmp(args[argc-1], "&") == 0) { printf("[%d]\n", pid); } //background process
		else { 
			waitpid(pid, &status, WUNTRACED); //not a background process => wait for completion
		} 
	} 
		
	return 1;
}

int run_command(char **args) {
	
	if(argc == 0 || args[0] == NULL) { return -1; } 
	
	if(strcmp(args[0], "cd") == 0) {
;		return run_cd(args);
	}
	else if(strcmp(args[0], "exit") == 0) {
		return 200; //OK
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
	args[pos] = NULL; //to detect last element, e.g. used in run_cd() to check for args
	return args;
}

char *readinput() {
	
	/**size_t bufsize = 512;
	char *buf = malloc(sizeof(char)*bufsize);
	if(!buf) { fprintf(stderr, "Failed to allocate memory. Exiting."); exit(10); }
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
	}
	buf[pos] = '\0';
	return buf;**/
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
	int ret = 0;
	
	while(1) {
		pipe_index = 0;	//resetting pipe_index
		printf("%s/> ", relative_path());
		args = split(readinput());
		ret = run_command(args);
		
		free(args);
		
		if(ret == 200) {
			free(cwd);
			free(entry);
			exit(200);
		}
	}
	
}

// exit codes: 3 

int main(void) {
	get_cwd("entry"); //get entry point
	get_cwd("current"); //get current path
	processes = NULL;
	
	//welcome message
	printf("HELLO SHELL\n");
	
	shell_loop(); //run shell in a loop
	
	return 0;
}
