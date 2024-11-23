#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* https://www.youtube.com/watch?v=gaXigSu72A4 */
static inline void die(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	int pdes[2];
	pid_t child;
 	
	if (pipe(pdes) < 0)
		die("pipe()");
 	
	if ((child = fork()) < 0)
		die("fork()");
 	
	if (child == 0) {
		/* child process */
      	close(pdes[0]);
		close(1);       /* close stdout */
		
		if (dup(pdes[1]) < 0)
			die("dup()");
		
		/* now stdout and pdes[1] are equivalent (dup returns lowest free descriptor) */
		if (execlp("cat", "cat", "/etc/passwd", NULL) < 0)
			die("execlp()");
		
		exit(EXIT_SUCCESS);
	}
	else {
		/* parent process */
		close(0);       /* close stdin */
		close(pdes[1]);
		
		if (dup(pdes[0]) < 0)
			die("dup()");
		
		/* now stdin and pdes[0] are equivalent (dup returns lowest free descriptor) */
		if (execlp("wc", "wc", "-l", NULL) < 0)
			die("execlp()");
		
		exit(EXIT_SUCCESS);
	}
	
	return 0;
}
