#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h> /*struct storing addr info*/
#include <sys/socket.h> /*socket api*/
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 9000
#define HOST "127.0.0.1"

int main(int argc, char const* argv[]){
	/*create server socket similar to client socket*/
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);

	/*data sent to client*/
	char serMsg[255] = "Message from Server to Client \'Hello Client\' ";

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

	return 0;
}