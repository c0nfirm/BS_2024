#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define HOST "127.0.0.1"

/*Error Catch*/
static inline void die(const char *msg){
	perror(msg);
	exit(-1);
}

int main(){
	/*connection to server*/
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr.s_addr = inet_addr(HOST)
	};

	char buf[256];
	int cfd;

	if((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket!");
	
	if(connect(cfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		die("Couldn't connect to socket!");

	/*commands and den server*/
	for(int i = 0; i < 5; i++){
		if(write(cfd, "Ping", 4) < 0)
			die("Couldn't send message!");
		printf("[send] Ping\n");

		if(read(cfd, buf, sizeof(buf)) < 0)
			die("Couldn't recieve message!");
		printf("[recv] %s\n", buf);		
	}	
	
	close(cfd);
	return 0;
}

int shell_exit();
/*Client Put*/
int c_put(char **args);
/*Client Get*/
int c_get(char **args);
/*start server connection*/
int launch(char **args, int argcount);
/*reads input line from stdin and returns it*/
char *read_line(void);
/*  Splits the input line into tokens
    returns a null terminated array of tokens
*/
char **split_line(char *line, int *argcount);
/*shell commands*/
int run(char **args, int argccount);