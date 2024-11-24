#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9000

/*Server data structure: keeps track of how many workers are
	active, and whether a shutdown has been requested*/
typedef struct{
	int num_workers;
	int shutdown_requested;

	clock_t last_message_timestamp;
	char last_message[1024];

	pthread_mutex_t lock;
	pthread_cond_t cond;
} Server;

/*Data used by a worker that chats with a specific client*/
typedef struct{
	Server *s;
	int client_fd;
} Worker;

/*Server task - waits for incoming connections, and spawns workers*/
void *server(void *arg);
/*Worker task - chats with clients*/
void *worker(void *arg);

static inline void die(const char *msg){	
	perror(msg);
	exit(-1);
}


/*reads input line from stdin and returns it*/
char *read_line(void);
/*  Splits the input line into tokens
    returns a null terminated array of tokens
*/
char **split_line(char *line);

/*         Change Directory Command
    args[0] =  "cd", args[1] = directory
    returns 1 to continue execution
*/
int shell_cd(char **args);
/*shell launch*/
int launch(char **args);

/*Server - Get*/	
int s_get(char **args, FILE *fh);
/*Server - Put*/
int s_put(char **args, FILE *fh);
int s_wall(char **args, Server *s);

/Server - Commands/
int execute(char **args, FILE *write_fh, FILE *read_fh, Server *s);

void *server(void *arg);
void *server(void *arg);

int main(int argc, char **argv) {
	while (1) {
		printf("[srv]: idle\n");
		sleep(2);
	}
	
	return 0;
}
