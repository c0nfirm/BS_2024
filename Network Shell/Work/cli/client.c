#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h> /*structure for storing address info*/
#include <sys/socket.h> /*socket api*/
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 9000
#define HOST "127.0.0.1"

int main(int argc, char const* argv[]){
	int sockD = socket(AF_INET, SOCK_STREAM, 0); /*client socket (Domain, Type, IP)*/

	struct sockaddr_in serv_addr;	/*connection address*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);	/*server port*/
	serv_addr.sin_addr.s_addr = inet_addr(HOST);

	/*trying to connect*/
	int connectStatus = connect(sockD, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if(connectStatus == -1){printf("Error...\n");}
	
	/*command to server | recv = rectrieve data*/
	else{
		char strData[255];
		recv(sockD, strData, sizeof(strData), 0);
		printf("Message: %s\n", strData);
	}
	
	return 0;
}


/*TODO - focus s/c verhalten
CD <Pfad>- wechshel des aktuellen verzeichnisses
EXIT - disconnect client <> server
GET <Pfad> - dl Datei < server
PUT <Pfad> - ul Datei > server*/