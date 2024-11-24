#include <stdio.h>
#include <stdlib.h>

#define PORT 9000
#define HOST "127.0.0.1"

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

static inline void die(const char *msg){
	perror(msg);
	exit(-1);
}

int main() {
	getc(stdin);
	printf("Client Exit\n");
	
	return 0;
}
