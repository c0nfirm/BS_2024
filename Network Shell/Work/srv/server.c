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
		if(strncmp("ls\n",buf, 3) == 0){
			printf("IN LS\n");
			/*execute linux cmd ls*/
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
			bzero(buf, MAX);
			strcpy(buf, "put");
			send(c_socked, "successful", sizeof(buf), 0);
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
		printf("&> test_bf_func\n");
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

	/*listen for connections
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
	func(c_socket);*/

	close(servSockD);
}


