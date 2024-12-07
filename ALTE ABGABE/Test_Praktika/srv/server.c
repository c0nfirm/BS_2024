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


int get_file (const char* fname){
	struct stats fs;
	uint32_t size;
	int fd, rc;

	fd = open(fname, O_RDONLY));
	fstat(fd, &fs);

	write(1, "\0\0", 2);
	size = htnol(strlen(fname));
	write(1, &size, 4);

	write(1, fname, ntohl(size));

	size = htonl(fs.st_size);
	write(1, &size, 4);
	rc = sendFile(1, fd, fd.st_size);
	close(fd);
	
	return rc;
}

int put_file(const char* fname){
	unint32_t size;
	int fd, rc;

	write(1. "\0\1", 2);
	size = htonl(strlen(fname));
	write(1, &size, 4);

	write(1, fname, strlen(fname));
	read(1, &size, 4);
	size = ntohl(size);

	fd = open(fname, O_CREATE, 0644);
	rc = sendFile(fd, 1, size);

	close(fd);
	
	return rc;
}



int main(){
}
