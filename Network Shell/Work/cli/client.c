#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>		/*read() Write() close()*/
#include <netinet/in.h> /*structure for storing address info*/
#include <sys/socket.h> /*socket api*/
#include <sys/types.h>
#include <arpa/inet.h>	/*inet_addr()*/
#include <strings.h> 	/*bzero()*/
#include <string.h>

#define PORT 9000
#define HOST "127.0.0.1"
#define MAx 128

void func (int sockD){
	char buf[MAx];
	int n;

	/*infinite loop*/
	for(;;){
		/*clears any data in buf by overwriting it with zero "\0" */
		bzero(buf, sizeof(buf));
		printf("$> ");
		n = 0;
		while((buf[n++] = getchar()) != '\n');
		
		write(sockD, buf, sizeof(buf));
		bzero(buf, sizeof(buf));
		read(sockD, buf, sizeof(buf));

		printf("From Server %s", buf);

		if((strncmp(buf, "exit", 4)) == 0){
			printf("Client Exit...\n");
			break;
		}

	}
}

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
	
	/*usr input relayed client -> server*/
	func(sockD);

	/*close the socket*/
	close(sockD);
}


/*TODO - focus s/c verhalten
CD <Pfad>- wechshel des aktuellen verzeichnisses
EXIT - disconnect client <> server
GET <Pfad> - dl Datei < server
PUT <Pfad> - ul Datei > server*/