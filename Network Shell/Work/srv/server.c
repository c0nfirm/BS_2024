#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h>

#define PORT 9000

/*Error Catch*/
static inline void die(const char *msg){	
	perror(msg);
	exit(-1);
}

int main(){
	struct sockaddr_in s_addr, c_addr;
	int sockopt = 1;
	socklen_t sad_sz = sizeof(struct sockaddr_in);
	int sfd, cfd;
	ssize_t bytes;
	char buf[256];

	/*Connection handling between s and c*/
	s_addr.sin_family =AF_INET;
	s_addr.sin_port = htons(PORT);
	s_addr.sin_addr.s_addr = INADDR_ANY;

	if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket!");
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &sockopt, sizeof(sockopt));

	if(bind(sfd, (struct sockaddr*) &s_addr, sad_sz) < 0)
		die("Couldn't bind socket!");

	if(listen(sfd, 1) < 0)
		die("Couldn't listen to the socket!");

	cfd = accept(sfd, (struct sockaddr*) &c_addr, &sad_sz);
	if(cfd < 0)
		die("Couldn't accept incoming connection!");

	/*"Execute" client queries*/
	while((bytes = read(cfd, buf, sizeof(buf))) != 0){
		if(bytes < 0)
			die("Couldn't recieve message!");

		if(write(cfd, buf, bytes) < 0)
			die("Couldn't send message!");	
	}

	close(cfd);
	close(sfd);				
	return 0;
}

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

/*Server - Commands*/
int execute(char **args, FILE *write_fh, FILE *read_fh, Server *s);

void *server(void *arg);
void *server(void *arg);
