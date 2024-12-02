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
#define SIZE 1024

/*void s_writeF(int sockFD){
	int n;
	FILE *fp;
	char name[MAX], path[MAX], cuurr_dir[MAX], *tmp = "test.txt";
	char data[SIZE];
	printf("[-] write func\n");
	fp = fopen(tmp, "wb");
	printf("[-] fopen\n");
	printf("[-] TEST_1\n");
	while(1){	
		printf("[-] TEST_3\n");
		recv(sockFD, data, sizeof(data), 0);
		printf("[-]Data : %s\n", data);
		if(fputs(data, fp)== EOF){break;}
		printf("[-]recv = %i\n", n);
		if(n <= 0){
			printf("[-] FINISH\n");
			break;
		}
		printf("[-] TEST_4\n");
		fprintf(fp, "%s", data);
		bzero(data, SIZE);
	}
	fclose(fp);
	printf("File recieved\n");
}*/

void func(int c_socked){
	printf("test_func_start\n");
	char buf[MAX], tmp[MAX], home_dir[MAX];
	FILE *out;
	if(getcwd(home_dir, MAX) == 0){}
	
	/*infinite loop*/
	for(;;){
		/*clears any data in buf by overwriting it with zero "\0" */
		bzero(buf, MAX);
		/*read msg from client*/
		recv(c_socked, buf, sizeof(buf), 0);
		printf("FIRST RECV: %s\n", buf);

		/* ls - command from client*/
		if(strncmp("ls\n",buf, 3) == 0 || memcmp("ls ", buf, 3) == 0){
			printf("IN LS\n");
			out = popen("ls -a", "r");

			bzero(buf, MAX);
			if(out == NULL){fputs("POPEN: Failed to execute command ls.\n", stderr);}
			else{
				printf("ELSE LS\n");
				while(fgets(tmp, MAX-1, out) != NULL){
					strcat(buf, tmp);
				}
			}
			pclose(out);
			send(c_socked, buf, sizeof(buf), 0);
			continue;
		}
		/* cd <path> - command from client*/
		if(memcmp("cd ",buf, 3) == 0 || memcmp("cd\n", buf, 3) == 0){
			printf("NORMAL CD\n");
			char path[MAX];
			strcpy(path, buf);
			size_t len = strlen(path);
			memmove(path, path+3, len - 3 + 1);
			printf("abgeschnitten: %s\n", path);
			strtok(path, "\n");

			bzero(buf, MAX);

			if(chdir(path) == 0){
				if(getcwd(buf, MAX) != NULL){
					printf("cwd: %s\n", buf);
					strcat(buf, "\n");
					send(c_socked, buf, sizeof(buf), 0);
				}
			}else{
				if(chdir(home_dir) == 0){
					if(getcwd(buf, MAX) != NULL){
						printf("cwd: %s\n", buf);
						strcat(buf, "\n");
						send(c_socked, buf, sizeof(buf), 0);
					}
				}
			}
			continue;
		}
		/* get <file> - command from client*/
		if(strncmp("get ",buf, 4) == 0){
  			bzero(buf, MAX);
			strcpy(buf, "get");
			send(c_socked, buf, sizeof(buf), 0);
			continue;
		}
		/* put <file> - command from client*/
		if(strncmp("put ",buf, 4) == 0){
			printf("[-] buf: %s\n", buf);
			FILE *fd;
			char path[MAX], *tmp = "text.txt";
			fd = fopen(tmp, "w");
			strcpy(path, buf);
			printf("[-] path: %s\n", path);
			size_t len = strlen(path);
			memmove(path, path+4, len - 4 + 1);
			printf("abgeschnitten: %s\n", path);
			strtok(path, "\n");
			printf("[-] buf: %s\n", path);
			send(c_socked, path, sizeof(path), 0);
			bzero(buf, MAX);

			while (1){
				recv(c_socked, buf, sizeof(buf), 0);
				printf("[-]Buf : %s\n", buf);
				fprintf(fd, "%s", buf);
			}
			printf("File recieved\n");
			continue;
		}
		/* ./<prog> - command from client*/
		if(strncmp("./",buf, 2) == 0){
			bzero(buf, MAX);
			strcpy(buf, "./<prog>");
			send(c_socked, buf, sizeof(buf), 0);
			continue;
		}
		/* exit - command from client*/
		if(strncmp("exit\n",buf, 5) == 0){
			bzero(buf, MAX);
			strcpy(buf, "exit");
			send(c_socked, buf, sizeof(buf), 0);
			break;
		}else{
			bzero(buf, MAX);
			strcpy(buf, "Falsche oder Leere eingabe!\n");
			send(c_socked, buf, sizeof(buf), 0);
		}
	}
}

int main(int argc, char const* argv[]){
	/*create server socket similar to client socket*/
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);
	
	/*server console input*/
	char m_buf[MAX];
	int n;

	/*data sent to client*/
	char serMsg[255] = " \'Hello Client\' ", strData[255];
	

	/*define server addr*/
	struct sockaddr_in serv_Addr;
	serv_Addr.sin_family = AF_INET;
	serv_Addr.sin_port = htons(PORT);
	serv_Addr.sin_addr.s_addr = inet_addr(HOST);

	/*bind socket to specified IP and Port*/
	bind(servSockD, (struct sockaddr*) &serv_Addr, sizeof(serv_Addr));

	for(;;){
		/*listen for connections*/
		printf("Waiting for connection...\n");
		listen(servSockD, 1);
		
	
		/*INT to hold client socket*/
		int c_socket = accept(servSockD, NULL, NULL);

		/*send sever msg to client socket*/
		send(c_socket, serMsg, sizeof(serMsg), 0);

		/*command to server | recv = rectrieve data*/
		recv(c_socket, strData, sizeof(strData), 0);
		printf("Message: %s\n", strData);

		/*server --> client functions*/
		printf("test_bf_func\n");
		func(c_socket);

		printf("If you want to close the server type 'exit'!\n");
		printf("Else enter any Input\n");
		n = 0;
		while((m_buf[n++] = getchar()) != '\n');
		if(strncmp("exit\n",m_buf, 5) == 0){
			printf("Server Closed!\n");
			bzero(m_buf, MAX);
			break;
		}
	}
	close(servSockD);
}