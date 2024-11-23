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
	char * line = malloc(100);
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


int main(void) {
	get_cwd("entry"); //get entry point
	get_cwd("current"); //get current path
	processes = NULL;
	
	//welcome message
	printf("HELLO SHELL\n");
	
	shell_loop(); //run shell in a loop
	
	return 0;
}
