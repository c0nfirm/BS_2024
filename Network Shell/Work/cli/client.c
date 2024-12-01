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
#define MAX 128

void func (int sockD){
	char buf[MAX];
	int n;

	/*infinite loop*/
	for(;;){
		printf("$> ");
		/*clears any data in buf by overwriting it with zero "\0" */
		bzero(buf, sizeof(buf));
		
		n = 0;
		while((buf[n++] = getchar()) != '\n');
		
		//write(sockD, buf, sizeof(buf));
		send(sockD, buf, sizeof(buf), 0);
		printf("Msg to sev: %s", buf);
		bzero(buf, sizeof(buf));
		
		recv(sockD, buf, sizeof(buf), 0);
		printf("From Server: %s", buf);

		if((strncmp(buf, "exit", 4)) == 0){
			printf("Client Exit...\n");
			break;
		}
	}
}

int main(int argc, char const* argv[]){
	/*client socket (Domain, Type, IP)*/
	int sockD = socket(AF_INET, SOCK_STREAM, 0);

	/*data sent to client*/
	char cliMsg[255] = " \'Hello server\' ";
	
	struct sockaddr_in serv_addr;	/*connection address*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);	/*server port*/
	serv_addr.sin_addr.s_addr = inet_addr(HOST);

	/*trying to connect*/
	int connectStatus = connect(sockD, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if(connectStatus == -1){
		printf("Error...\n");
	}else{
		char strData[255];
		/*command to server | recv = rectrieve data*/
		recv(sockD, strData, sizeof(strData), 0);
		printf("Message: %s\n", strData);
		
		/*send sever msg to client socket*/
		send(sockD, cliMsg, sizeof(cliMsg), 0)
		
		;/*usr input relayed client -> server*/
		func(sockD);
	}
	
	/*close the socket*/
	close(sockD);
}


/*TODO - focus s/c verhalten
CD <Pfad>- wechshel des aktuellen verzeichnisses
EXIT - disconnect client <> server
GET <Pfad> - dl Datei < server
PUT <Pfad> - ul Datei > server*/