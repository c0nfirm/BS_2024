#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
	pid_t proc_id;
	int status = 0;
	
	proc_id = fork();
	
	if (proc_id < 0) {
		fprintf(stderr, "fork error\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	
	if (proc_id == 0) {
		/* child process */
		printf("[child]  process id: %d\n", (int) getpid());
		
		char* args[] = {"sleep", "1", NULL};
		execvp(args[0], args);
		exit(-1);
	}
	else {
		/* parent */
		printf("[parent] process id: %d\n", (int) getpid());
		pid_t child_id = wait(&status);
		
		printf("[parent] child %d returned: %d\n",
		        child_id, WEXITSTATUS(status));
	}
	
	return EXIT_SUCCESS;
}
