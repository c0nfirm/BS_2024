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
/*file size placeholder! Maybe get filesize and allocate it on the fly*/
#define FMAX 1024

void read_f(char *f_name, int c_cocked){
	FILE *fd;
	fd = fopen(f_name, "r"); /*read*/
	char f_buf[FMAX];
	if(fd == NULL){
		printf("[-]Error: Could not open File!\n");
		exit(-1);
	}

	bzero(f_buf, FMAX);
	/*reads file line and send's it to the client*/
	while(fgets(f_buf, FMAX, fd) != NULL){
		printf("[+] bf_buf:  %s\n", f_buf);
		send(c_cocked, f_buf, sizeof(FMAX), 0);
		printf("[+] af_buf:  %s\n", f_buf);
	}
	fclose(fd);
}

void write_f(char *f_name, FILE *fd, int c_socked){
	fd = fopen(f_name, "w"); /*write*/
	char f_buf[FMAX];
	if(fd == NULL){
		printf("[-]Error: Could not open File!\n");
		exit(-1);
	}

	for(;;){
		recv(c_socked, f_buf, sizeof(FMAX), 0);
		if(fprintf(fd,"%s", f_buf) < 0){
			break;
		}
	}
	fclose(fd);
}

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

		/*catch put*/
		if(memcmp("put ", buf,  4) == 0){
			/*send imput to server*/
			send(sockD, buf, sizeof(buf), 0);
			printf("Msg to sev: %s", buf);
			bzero(buf, MAX);
			
			/*gets's filename to upload to server*/
			recv(sockD, buf, sizeof(buf), 0);
			read_f(buf, sockD);
			bzero(buf, MAX);

			continue;
		}

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
GET <Pfad> - dl Datei < server
PUT <Pfad> - ul Datei > server
<prog ausführen>*/