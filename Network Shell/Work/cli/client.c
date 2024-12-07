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
#define FMAX 1024

/*unused way to get file size
long get_file_size(char *filename){
	FILE *fp = fopen(filename, "r");
	if (fp==NULL)
		return -1;
	
	if (fseek(fp, 0, SEEK_END) < 0) {
		fclose(fp);
		return -1;
	}

	long size = ftell(fp);

	fclose(fp);
	return size;
}*/

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
		send(c_cocked, f_buf, sizeof(f_buf), 0);
	}
	send(c_cocked, "\0", 1, 0);
	fclose(fd);
}

void write_f(char *f_name, int c_socked){
	FILE *fd;
	char f_buf[FMAX];
	fd = fopen(f_name, "w"); /*write*/
	bzero(f_buf, FMAX);

	if(fd == NULL){
		printf("[-]Error: Could not open File!\n");
		exit(-1);
	}

	for(;;){
		recv(c_socked, f_buf, sizeof(f_buf), 0);
		if(memcmp("\0", f_buf, 1) == 0){
			break;
		}
		if(fprintf(fd,"%s", f_buf) < 0){
			break;
		}
			
		bzero(f_buf, FMAX);
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
			/*send input to server*/
			send(sockD, buf, sizeof(buf), 0);
			//printf("Msg to sev: %s", buf);
			bzero(buf, MAX);
			
			/*gets's filename to upload to server*/
			recv(sockD, buf, sizeof(buf), 0);
			read_f(buf, sockD);
			bzero(buf, MAX);

			continue;
		}

		/*catch get*/
		if(memcmp("get ", buf,  4) == 0){
			/*receive input from server*/
			send(sockD, buf, sizeof(buf), 0);
			//printf("Msg to sev: %s", buf);
			bzero(buf, MAX);
			
			/*gets's filename to upload to server*/
			recv(sockD, buf, sizeof(buf), 0);
			write_f(buf, sockD);
			bzero(buf, MAX);

			continue;
		}

		//write(sockD, buf, sizeof(buf));
		send(sockD, buf, sizeof(buf), 0);
		//printf("Msg to sev: %s", buf);
		bzero(buf, sizeof(buf));
		
		recv(sockD, buf, sizeof(buf), 0);
		//printf("From Server: %s", buf);
		if((strncmp(buf, "exit", 4)) == 0){
			printf("Client Exit...\n");
			break;
		}
		printf("%s", buf);
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
		//printf("Message: %s\n", strData);
		
		/*send sever msg to client socket*/
		send(sockD, cliMsg, sizeof(cliMsg), 0)
		
		;/*usr input relayed client -> server*/
		func(sockD);
	}
	
	/*close the socket*/
	close(sockD);
}
