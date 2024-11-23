#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, const char* argv[]) {
	pid_t pid;
	int h;
	int count;
	
	if (argc < 2) {
		printf("Usage: ./fork process#\n");
		return 0;
	}
	
	count = atoi(argv[1]) - 1;
	printf("Process#: %d\n", count + 1);
	printf("Parent started\n");
	
	pid = 1;
	for (h = 0; h < count; ++h) {
		if ((pid = fork()) < 0)
			printf("fork error\n");
		else if (pid == 0) {
			printf("Child %d started\n", h);
			sleep(1);
			printf("Child %d finished\n", h);
			exit(0);
		}
	}
	
	printf("Parent waiting\n");
	for (h = 0; h < count; ++h) {
		wait(NULL);
	}
	printf("Parent finished\n");
	return EXIT_SUCCESS;
}
