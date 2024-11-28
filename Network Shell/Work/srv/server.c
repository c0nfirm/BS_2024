#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h> /*struct storing addr info*/
#include <sys/socket.h> /*socket api*/
#include <sys/types.h>
#include <arpa/inet.h>	/*inet_addr()*/
#include <unistd.h>		/*read(), write(), close()*/
#include <string.h>

#define PORT 9000
#define HOST "127.0.0.1"
#define MAX 128

void func(int c_socked){
	printf("&> test_func_start");
	char buf[MAX];
	//int n;
	

	/*infinite loop*/
	for(;;){
		/*clears any data in buf by overwriting it with zero "\0" */
		bzero(buf, MAX);
		/*read msg from client*/
		//read(c_socked, buf, sizeof(buf));

		/* cd <path> - command from client*/
		if(strncmp("cd ",buf, 3) == 0){
			printf("&> Client send 'cd'");
		}
		/* get <file> - command from client*/
		if(strncmp("get ",buf, 4) == 0){
			printf("&> Client send 'get'");
		}
		/* put <file> - command from client*/
		if(strncmp("put ",buf, 4) == 0){
			printf("&> Client send 'put'");
		}
		/* ./<prog> - command from client*/
		if(strncmp("./ ",buf, 3) == 0){
			printf("&> Client send './'");
		}
		/* exit - command from client*/
		if(strncmp("exit ",buf, 5) == 0){
			printf("&> Client send 'exit'");
		}

		/*print buf (client contents)
		printf("&> %s", buf);
		clears any data in buf by overwriting it with zero "\0"
		bzero(buf, MAX);
		n = 0;
		server msg --> buf
		while((buf[n++] = getchar()) != '\n');
		send buf ---> client
		write(c_socked, buf, sizeof(buf));
		msg "exit" = server closed + chat end
		if(strncmp("exit", buf, 4) == 0){
			printf("Server Exit...\n");
			break;
		}*/
	}
}

int main(int argc, char const* argv[]){
	/*create server socket similar to client socket*/
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);

	/*data sent to client*/
	char serMsg[255] = " \'Hello Client\' ";

	/*define server addr*/
	struct sockaddr_in serv_Addr;
	serv_Addr.sin_family = AF_INET;
	serv_Addr.sin_port = htons(PORT);
	serv_Addr.sin_addr.s_addr = inet_addr(HOST);

	/*bind socket to specified IP and Port*/
	bind(servSockD, (struct sockaddr*) &serv_Addr, sizeof(serv_Addr));

	/*listen for connections*/
	listen(servSockD, 1);

	/*INT to hold client socket*/
	int c_socket = accept(servSockD, NULL, NULL);

	/*send sever msg to client socket*/
	send(c_socket, serMsg, sizeof(serMsg), 0);

	/*command to server | recv = rectrieve data*/
	char strData[255];
	recv(c_socket, strData, sizeof(strData), 0);
	printf("Message: %s\n", strData);
	
	/*server --> client functions*/
	printf("&> test_bf_func");
	func(c_socket);

	close(servSockD);
}