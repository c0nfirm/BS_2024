
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static inline void die(const char* msg)
{
	perror(msg);
	exit(-1);
}

int main()
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(8000),
		.sin_addr.s_addr = inet_addr("127.0.0.1")
	};
	char buf[256];
	int cfd;
	
	if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Couldn't open the socket");
	
	if (connect(cfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		die("Couldn't connect to socket");
	
	for (int i = 0; i < 5; ++i)
	{
		if (write(cfd, "Ping", 4) < 0)
			die("Couldn't send message");
		
		printf("[send] Ping\n");
		
		if (read(cfd, buf, sizeof(buf)) < 0)
			die("Couldn't receive message");
		
		printf("[recv] %s\n", buf);
	}
	
	close(cfd);
	return 0;
}
