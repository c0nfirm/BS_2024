#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>

#include <netinet/in.h> /*for struct sockaddr_in*/
#include <arpa/inet.h>  /*for inet_pton*/

#include <string.h>

#define BUFSIZE 2000
#define PORT 9000

#define SHELL_KEY_BUFF_SIZE 64
#define SHELL_KEY_DELIM "\t,\r,\n,\a, "
#define SHELL_BUFF_SIZE 1024

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
char *read_line(void){
	int buff_size = SHELL_BUFF_SIZE;
	int posi = 0;
	char *buff = malloc(sizeof(char) * buff_size);
	int c;

	if (!buff){
		fprintf(stderr, "Shell: Allocation Error!\n");
		exit(EXIT_FAILURE);
	}

	while (1){
		/*read a character*/
		c = getchar();

		/*when EOL, replace it with null character and return*/
		if (c == EOF || c == '\n'){
			buff[posi] = '\0';
			return buff;
		}else{buff[posi] = c;}

		posi++;

		/*reallocate it buffer is exceeded*/
		if (posi >= buff_size){
			buff_size += SHELL_BUFF_SIZE;
			buff = realloc(buff, buff_size);
			if (!buff){
				fprintf(stderr, "Shell: Allocation Error!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

/*  Splits the input line into tokens
    returns a null terminated array of tokens
*/
char **split_line(char *line){
	int buff_size = SHELL_KEY_BUFF_SIZE, posi = 0;
	char **keys = malloc(buff_size * sizeof(char *));
	char *key;

	key = strtok(line, SHELL_KEY_DELIM);
	while (key != NULL){
		keys[posi] = key;
		posi++;

		if (posi >= buff_size){
			buff_size += SHELL_KEY_BUFF_SIZE;
			keys = realloc(keys, buff_size * sizeof(char *));
			if (!keys){
				fprintf(stderr, "Allocation Error!\n");
				exit(EXIT_FAILURE);
			}
		}

		key = strtok(NULL, SHELL_KEY_DELIM);
	}

	keys[posi] = NULL;
	return keys;
}

/*         Change Directory Command
    args[0] =  "cd", args[1] = directory
    returns 1 to continue execution
*/
int shell_cd(char **args){
	if (args[1] == NULL){
		fprintf(stderr, "Shell: expected argument to \"cd\"\n");
	}else{
		if (chdir(args[1]) != 0){
			perror("Shell");
		}
	}

	return 1;
}

/*launches the shell*/
int launch(char **args){
	pid_t pid;
	int state;
	pid = fork();

	if (pid == 0){
		/*Child Process*/
		if (execvp(args[0], args) == -1){ 	 /*path dir*/
			if (execv(args[0], args) == -1){ /*current dir*/
			
				perror("Shellchild");
			}
		}

		exit(EXIT_FAILURE);
	}else{
		if (pid < 0){
			/*Forking Error*/
			perror("Shell");
		}else{
			/*Parent Process*/
			do{waitpid(pid, &state, WUNTRACED);
			} while (!WIFEXITED(state) && !WIFSIGNALED(state));
		}
	}
	return 1;
}

int server_get(char **args, FILE *fh){
	if (args[1] == NULL){
		fprintf(stderr, "Need FileName \n");
		/*empty command was entered*/
		return 1;
	}

	struct sockaddr_in srv_addr, cli_addr;
	int sockopt = 1;
	socklen_t sad_sz = sizeof(struct sockaddr_in);
	int sfd, cfd;

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(PORT + 1);
	srv_addr.sin_addr.s_addr = INADDR_ANY;

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");

	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

	if (bind(sfd, (struct sockaddr *)&srv_addr, sad_sz) < 0)
		die("Couldn't bind socket");

	if (listen(sfd, 1) < 0)
		die("Couldn't listen to the socket");

	cfd = accept(sfd, (struct sockaddr *)&cli_addr, &sad_sz);
	if (cfd < 0)
		die("Couldn't accept incoming connection");

	int socket = sfd;
	fh = fdopen(socket, "w");
	FILE *file = fopen(args[1], "r");
	fseek(file, 0, SEEK_END); 		/*seek to end of file*/ 
	int size = ftell(file);  		/*get current file pointer*/ 
	fseek(file, 0, SEEK_SET);	 	/*seek back to beginning of file*/ 
	fprintf(fh, "%i\r\n", size);	
	fflush(fh);
	int filebuffer;
	for (int i = 0; i < size; i++){
		filebuffer = getc(file);
		putc(filebuffer, fh);
	}

	fflush(fh);
	return 1;
}

int server_put(char **args, FILE *fh){
	int length = atoi(args[2]);
	int filebuffer;
	FILE *file = fopen(args[1], "w");

	for (int i = 0; i < length;){
		filebuffer = getc(fh);
		if (filebuffer == EOF)
			continue;
		putc(filebuffer, file);
		i++;
	}

	fclose(file);
	return 1;
}

int server_wall(char **args, Server *s){
	pthread_mutex_lock(&s->lock);
	memcpy(s->last_message, args[1], 1024);
	s->last_message_timestamp = clock();
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->lock);
	return 1;
}

int execute(char **args, FILE *write_fh, FILE *read_fh, Server *s){
	if (args[0] == NULL){	/*empty command was entered*/
		return 1;
	}
	if (strcmp(args[0], "cd") == 0){
		return shell_cd(args);
	}
	else if (strcmp(args[0], "get") == 0){
		return server_get(args, write_fh);
	}
	else if (strcmp(args[0], "put") == 0){
		return server_put(args, read_fh);
	}
	else if (strcmp(args[0], "wall") == 0){
		return server_wall(args, s);
	}

	return launch(args);
}

void *server(void *arg){
	Server *s = arg;

	int server_sock_fd;

	server_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock_fd < 0){
		perror("Couldn't create socket");
		exit(1);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

	int rc = bind(server_sock_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));

	if (rc < 0){
		perror("Couldn't bind server socket");
		exit(1);
	}

	/*Make the server socket nonblocking*/
	fcntl(server_sock_fd, F_SETFL, O_NONBLOCK);

	/*Create a fd_set with the server socket file descriptor
	  (so we can use select to do a timed wait for incoming
					connections.)*/
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(server_sock_fd, &fds);

	rc = listen(server_sock_fd, 5);
	
	if (rc < 0){
		perror("Couldn't listen on server socket");
		exit(1);
	}

	int done = 0;
	while (!done){
		/*Use select to see if there are any incoming connections
		ready to accept*/

		/*Set a 1-second timeout*/
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/*We want to wait for just the server socket*/
		fd_set readyfds = fds;

		/*Preemptively increase the worker count*/
		pthread_mutex_lock(&s->lock);
		s->num_workers++;
		pthread_mutex_unlock(&s->lock);

		/*Wait until either a connection is received, or the timeout expires*/
		int rc = select(server_sock_fd + 1, &readyfds, NULL, NULL, &timeout);

		if (rc == 1){
			printf("connection opened");
			
			/*The server socket file descriptor became ready,
			so we can accept a connection without blocking*/
			int client_socket_fd;
			struct sockaddr_in client_addr;
			socklen_t client_addr_size = sizeof(client_addr);

			client_socket_fd = accept(
				server_sock_fd,
				(struct sockaddr *)&client_addr,
				&client_addr_size);
			if (client_socket_fd < 0){
				perror("Couldn't accept connection");
				exit(1);
			}

			/*Create Worker data structure (which tells the worker
			thread what it's client socket fd is, and
			gives it access to the Server struct)*/
			Worker *w = malloc(sizeof(Worker));
			w->s = s;
			w->client_fd = client_socket_fd;

			/*Start a worker thread (detached, since no other thread
						will wait for it to complete)*/
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			pthread_t thr;
			pthread_create(&thr, &attr, &worker, w);
		}else{
			/*The select timed out.*/

			/*Decrement the worker count (since worker wasn't
					actually started.)*/
			pthread_mutex_lock(&s->lock);
			s->num_workers--;
			pthread_cond_broadcast(&s->cond);
			pthread_mutex_unlock(&s->lock);

			/*See if a shutdown was requested, and if so, stop
						waiting for connections.*/
			pthread_mutex_lock(&s->lock);
			if (s->shutdown_requested){
				done = 1;
			}
			pthread_mutex_unlock(&s->lock);
		}
	}

	close(server_sock_fd);

	return NULL;
}

void *worker(void *arg){
	Worker *w = arg;
	Server *s = w->s;

	char buf[BUFSIZE];
	char **args;
	int read_fd = w->client_fd;
	int write_fd = dup(w->client_fd); /*duplicate the file descriptor*/

	/*Wrap the socket file descriptor using FILE* file handles,
					for both reading and writing.*/
	FILE *read_fh = fdopen(read_fd, "r");
	FILE *write_fh = fdopen(write_fd, "w");

	int done = 0;
	clock_t last = clock();
	do{
		if (s->last_message_timestamp > last){ /*could break after 36min*/
			FILE *write_fh = fdopen(write_fd, "w");
			fprintf(write_fh, "%s\n", s->last_message);
			fflush(write_fh);
			last = clock();
		}

		/*Read one line from the client*/
		if (!fgets(buf, BUFSIZE, read_fh)){
			continue; // connection was interrupted?
		}

		/*Send message back to the client*/
		dup2(write_fd, STDOUT_FILENO);
		dup2(write_fd, STDERR_FILENO);

		if (strcmp(buf, "exit") == 0){
			done = 1;
		}else if (strcmp(buf, "shutdown") == 0){
			done = 1;
			printf("shutdown");
			// A shutdown was requested
			pthread_mutex_lock(&s->lock);
			s->shutdown_requested = 1;
			pthread_cond_broadcast(&s->cond);
			pthread_mutex_unlock(&s->lock);
		}

		args = split_line(buf);
		execute(args, write_fh, read_fh, s);
		sleep(1);
		printf("%s", buf);
	} while (done == 0);

	/*Close the file handles wrapping the client socket*/
	fclose(read_fh);
	fclose(write_fh);

	/*Free Worker struct*/
	free(w);

	/*Update worker count*/
	pthread_mutex_lock(&s->lock);
	s->num_workers--;
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->lock);

	return NULL;
}


int main(int argc, char **argv){
	Server *s = malloc(sizeof(Server));

	/*Initialize Server structure*/
	s->num_workers = 0;
	s->shutdown_requested = 0;
	s->last_message_timestamp = clock();
	pthread_mutex_init(&s->lock, NULL);
	pthread_cond_init(&s->cond, NULL);

	/*Start server thread*/
	pthread_t thr;
	pthread_create(&thr, NULL, &server, s);

	/*Wait for shutdown and for workers to finish*/
	pthread_mutex_lock(&s->lock);
	while (!s->shutdown_requested || s->num_workers > 0){
		pthread_cond_wait(&s->cond, &s->lock);
	}
	pthread_mutex_unlock(&s->lock);

	/*Join server thread*/
	pthread_join(thr, NULL);

	/*Free Server structure*/
	free(s);

	return 0;
}