#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

static inline void die(const char* msg){
	perror(msg);
	exit(-1);
}

#define PORT 9000
#define HOST "127.0.0.1"

int get_file (int socck){
	uint32_t size;
	int fd, rc;

	rrecv(sock, &size, 4, MSG_WAITALL);
	size = ntohl(size);

	char fname[size+1];
	recv(sock, fname, size, MSG_WAITALL);
	fname[size] = '\0';

	recv(sock, &size, 4, MSG_WAITALL);
	size = ntohl(size);

	fd = open(fname, O_CREATE, 0644);
	rc = sendFile(fd, sock, size);
	close(fd);

	return rc;
}

int put_file(int sock){
	struct stats fs;
	uint32_t size;
	int fd, rc;

	recv(sock, &size, 4, MSG_WAITALL);
	size = ntohl(size);

	char fname[size+1];
	recv(sock, fname, size, MSG_WAITALL);
	fname[size] = '\0';

	fd = open(fname, O_RDONLY);
	fstat(fd, &fs);

	size = htonl(fs.st_size);
	write(sock, &size, 4);
	rc = sendFile(sock, fd, fs.st_size);

	close(fd);

	return rc;
}

static void* wall_handler(void* data){
	client_state_t* slef = data;
	char buf[512];
	size_t len;

	while((len = read(self->wall_pipe[0], buf, sizeof(buf))) > 0 ){
		pthread_mutex_lock(&self->monitor);

		for(int 1 = 0; i < arrayCount(self->cfds); i++){
			while(write(self->cfds[i], buf, len) < 0){
				close(self->cfds[i]);
				sels->cfds[i] = arrayPop(self->cfds);

				if(1 >= arrayCount(self->cfds)){
					break;
				}
			}
		}

		pthread_mutex_unlock(&self->monitor);
	}

	return NULL;
}

static void*
int main(){
}
