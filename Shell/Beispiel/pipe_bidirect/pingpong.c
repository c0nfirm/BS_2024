#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

static int split(int fd[2]);
static int redir(int fd, const char* file);
static void sig_handler(int sig);

static int parent;

static inline void die(const char* msg) {
	perror(msg);
	exit(-1);
}

int main(void) {
	ssize_t bytes;
	char buf[256];
	int fd[2];
	
	// disable buffering for stdout
	setvbuf(stdout, NULL, _IONBF, 0);
	
	// redirect stdout to write into a file
	if (redir(1, "log.txt") < 0)
		die("redir()");
	
	// install the signal handler for Ctrl+C
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		die("signal()");
	
	// split processes and assign file descriptors for communication
	switch (split(fd)) {
	case -1: // error
		exit(-1);
		
	case 0:  // child
		while ((bytes = read(fd[0], buf, sizeof(buf))) > 0) {
			printf("[%u]: %s\n", getpid(), buf);
			sleep(1);
			if (write(fd[1], "pong", 5) < 0)
				die("write()");
		}
		break;
		
	case 1:  // parent
		parent = 1;
		if (write(fd[1], "ping", 5) < 0)
			die("write()");
		
		while ((bytes = read(fd[0], buf, sizeof(buf))) > 0) {
			printf("[%u]: %s\n", getpid(), buf);
			sleep(1);
			if (write(fd[1], "ping", 5) < 0)
				die("write()");
		}
		break;
	}
	
	// close the file descriptors
	close(fd[0]);
	close(fd[1]);
	
	return 0;
}

int split(int fd[2]) {
	int pfd[2][2];
	
	if (pipe(pfd[0]) < 0) {
		perror("pipe()");
		goto err_pipe1;
	}
	
	if (pipe(pfd[1]) < 0) {
		perror("pipe()");
		goto err_pipe2;
	}
	
	switch (fork()) {
	case -1:
		perror("fork()");
		goto err_fork;
		
	case 0:  // child process
		close(pfd[0][1]);  // close write for child
		close(pfd[1][0]);  // close read for parent
		fd[0] = pfd[0][0]; // keep read for child
		fd[1] = pfd[1][1]; // keep write for parent
		return 0;
		
	default: // parent process
		close(pfd[1][1]);  // close write for parent
		close(pfd[0][0]);  // close read for child
		fd[0] = pfd[1][0]; // keep read for parent
		fd[1] = pfd[0][1]; // keep write for child
		return 1;
	}
	
err_fork:
	close(pfd[1][0]);
	close(pfd[1][1]);
err_pipe2:
	close(pfd[0][0]);
	close(pfd[0][1]);
err_pipe1:
	return -1;
}

int redir(int fd, const char* file) {
	int fd2 = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	if (fd2 < 0)
		return -1;
	
	dup2(fd2, fd);
	close(fd2);
	
	return 0;
}

void sig_handler(int sig) {
	if (parent) {
		const char msg[] = "Programmabbruch durch Nutzer\n";
		exit(write(1, msg, sizeof(msg) - 1));
	}
	exit(0);
}
