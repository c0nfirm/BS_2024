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
	
	/*infinite loop*/
	for(;;){
		/*clears any data in buf by overwriting it with zero "\0" */
		bzero(buf, MAX);
		/*read msg from client*/
		recv(c_socked, buf, sizeof(buf), 0);

		/* cd <path> - command from client*/
		if(strncmp("cd ",buf, 3) == 0){
			printf("&> Client send 'cd'");
			bzero(buf, MAX);
			strcpy(buf, "cd <path>");
			send(c_socked, buf, sizeof(buf), 0);
		}
		/* get <file> - command from client*/
		if(strncmp("get ",buf, 4) == 0){
			printf("&> Client send 'get'");
			bzero(buf, MAX);
			strcpy(buf, "get <file>");
			send(c_socked, buf, sizeof(buf), 0);
		}
		/* put <file> - command from client*/
		if(strncmp("put ",buf, 4) == 0){
			printf("&> Client send 'put'");
			bzero(buf, MAX);
			strcpy(buf, "put <file>");
			send(c_socked, buf, sizeof(buf), 0);
		}
		/* ./<prog> - command from client*/
		if(strncmp("./",buf, 2) == 0){
			printf("&> Client send './'");
			bzero(buf, MAX);
			strcpy(buf, "./<prog>");
			send(c_socked, buf, sizeof(buf), 0);
		}
		/* exit - command from client*/
		if(strncmp("exit\n",buf, 5) == 0){
			printf("&> Client send 'exit'");
			bzero(buf, MAX);
			strcpy(buf, "exit");
			send(c_socked, buf, sizeof(buf), 0);
		}
	}
}

int main(int argc, char const* argv[]){
	/*create server socket similar to client socket*/
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);

	/*data sent to client*/
	char serMsg[255] = " \'Hello Client\' ", strData[255];
	//char m_buf[MAX];int n;

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
	recv(c_socket, strData, sizeof(strData), 0);
	printf("Message: %s\n", strData);
	
	/*server --> client functions*/
	printf("&> test_bf_func");
	func(c_socket);

	close(servSockD);
}


/*while(;;){
	listen for connections
	listen(servSockD, 1);
	
	INT to hold client socket
	int c_socket = accept(servSockD, NULL, NULL);

	send sever msg to client socket
	send(c_socket, serMsg, sizeof(serMsg), 0);

	command to server | recv = rectrieve data
	recv(c_socket, strData, sizeof(strData), 0);
	printf("Message: %s\n", strData);

	server --> client functions
	printf("&> test_bf_func");
	func(c_socket);

	n = 0;
		while((m_buf[n++] = getchar()) != '\n');
		if(strncmp("exit\n",m_buf, 5) == 0){
			printf("Server Closed");
			bzero(m_buf, MAX);
			break;
		}
}*/