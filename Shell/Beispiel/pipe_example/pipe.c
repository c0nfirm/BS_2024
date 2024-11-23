#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 32

int pipe_fds[2];

void term(const char *msg) {
	if (msg) {
		fprintf(stderr, "Error: %s\n", msg);
		fflush(stderr);
	}
	
	int close_fail = 0;
	int ret = close(pipe_fds[0]);
	
	if (ret < 0) {
		fprintf(stderr, "Error: Couldn't close reading end of pipe\n");
		fflush(stderr);
		close_fail = 1;
	}
	
	ret = close(pipe_fds[1]);
	
	if (ret < 0) {
		fprintf(stderr, "Error: Couldn't close writing end of pipe\n");
		fflush(stderr);
		close_fail = 1;
	}
	
	exit((msg || close_fail) ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(void) {
	int ret = pipe(pipe_fds);
	
	if (ret < 0)
		term("Couldn't create pipe");
	
	char out_buf[] = "hello";
	ret = write(pipe_fds[1], out_buf, strlen(out_buf) + 1);
	
	if (ret < 0)
		term("Couldn't write to pipe");
	
	printf("send msg: %s\n", out_buf);
	
	char in_buf[BUF_SIZE];
	memset(in_buf, 0, BUF_SIZE);
	
	ret = read(pipe_fds[0], in_buf, BUF_SIZE - 1);
	
	if (ret < 0)
		term("Couldn't read from pipe");
	
	printf("recv msg: %s\n", in_buf);
	return 0;
}
