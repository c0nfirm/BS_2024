#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 9000
#define HOST "127.0.0.1"

#define BUFSIZE 2000
#define SHELL_KEY_BUFF_SIZE 64
#define SHELL_KEY_DELIM "\t,\r,\n,\a, "
#define SHELL_BUFF_SIZE 1024
int s, state;
FILE *fh;

static inline void die(const char *msg){
    perror(msg);
    exit(-1);
}

int shell_exit(){
    fprintf(fh, "exit");
    fflush(fh);
    close(s);
    return 0;
}

/*Client PUT*/
int client_put(char **args){
    if (args[1] == NULL){                   /*empty command was entered*/
        fprintf(stderr, "Need Filename \n");
        return 1;
    }

    FILE *file = fopen(args[1], "r");
	fseek(file, 0, SEEK_END);               /*seek to end of file*/
	int size = ftell(file);	                /*get current file pointer*/
	fseek(file, 0, SEEK_SET);               /*seek back to beginning of file*/
    fprintf(fh, "%s ", args[0]);            /*get*/
    fprintf(fh, "%s ", args[1]);            /*dateiname*/
	fprintf(fh,"%i\r\n",size);              /*laenge*/
	fflush(fh);
    int filebuffer;
    
    for (int i = 0; i < size;i++){
        filebuffer = getc(file);
        putc(filebuffer, fh);
    }
    
    fflush(fh);
    return 1;
}

/*Client GET*/
int client_get(char **args){
    if (args[1] == NULL){                  /*empty command was entered*/
        fprintf(stderr, "Need Filename \n");
        return 1;
    }

    fprintf(fh, "%s", args[0]);           /*get*/
    fprintf(fh, "%s", args[1]);           /*dateiname*/
    fflush(fh);
    int fs, rc;
    fs = socket(AF_INET, SOCK_STREAM, 0);
   
    if (fs < 0){
        perror("Could not create socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT+1);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    rc = connect(fs, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
    if (rc < 0){
        perror("Could not connect to server");
        exit(1);
    }

    fcntl(fs, F_SETFL, O_NONBLOCK);
    
    FILE *read_fh = fdopen(fs, "r");
    char buf[BUFSIZE];
    
    while (1){
        if (!fgets(buf, BUFSIZE, read_fh)){  /*read size*/
            continue;
        }
    }

    int length = atoi(buf);
    int filebuffer;
    FILE *file = fopen(args[1], "w");

    for (int i = 0; i < length;){
        filebuffer = getc(read_fh);
        if (filebuffer == EOF)
            continue;
        putc(filebuffer, file);
        i++;
    }
    fclose(file);

    return 1;
}

/*launch server connection*/
int launch(char **args, int argcount){
    printf("sending to server\n");
    for (int i = 0; i < argcount; i++){
        fprintf(fh, "%s", args[i]);
        if ((argcount - i) >= 0){
            fprintf(fh, "%c", ' ');
        }
    }
    fflush(fh);
    return 1;
}

/*reads input line from stdin and returns it*/
char *read_line(void){
    int buff_size = SHELL_BUFF_SIZE;
    int posi = 0;
    char *buff = malloc(sizeof(char) * buff_size);
    int c;

    if (!buff){
        fprintf(stderr, "Shell: Allocation Error!\n");
        exit(EXIT_FAILURE);
    }

    while (1){
        c = getchar();   /*read a character*/

        /*when EOL, replace it with null character and return*/
        if (c == EOF || c == '\n'){
            buff[posi] = '\0';
            return buff;
        }else{buff[posi] = c;}

        posi++;

        /*reallocate if buffer is exceeded*/
        if (posi >= buff_size){
            buff_size += SHELL_BUFF_SIZE;
            buff = realloc(buff, buff_size);
            if (!buff){
                fprintf(stderr, "Shell: Allocation Error!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*  Splits the input line into tokens
    returns a null terminated array of tokens
*/
char **split_line(char *line, int *argcount){
    int buff_size = SHELL_KEY_BUFF_SIZE, posi = 0;
    char **keys = malloc(buff_size * sizeof(char *));
    char *key;

    key = strtok(line, SHELL_KEY_DELIM);
    while (key != NULL){
        keys[posi] = key;
        posi++;

        if (posi >= buff_size){
            buff_size += SHELL_KEY_BUFF_SIZE;
            keys = realloc(keys, buff_size * sizeof(char *));
            if (!keys){
                fprintf(stderr, "Allocation Error!\n");
                exit(EXIT_FAILURE);
            }
        }

        key = strtok(NULL, SHELL_KEY_DELIM);
    }

    keys[posi] = NULL;
    *argcount = posi;
    return keys;
}


int execute(char **args, int argcount){
    if (args[0] == NULL){
        /*empty command was entered*/
        return 1;
    }

    /*executes a specified shell command*/
    if (strcmp(args[0], "get") == 0){
        return client_get(args);
    }
    else if (strcmp(args[0], "exit") == 0){
        return shell_exit();
    }
    else if (strcmp(args[0], "put") == 0){
       return client_put(args);
    }

    return launch(args, argcount);
}

int main(){
    char *line;
    char **args;
    int state, argcount, rc;
    /*char buf[256];*/

    signal(SIGPIPE, SIG_IGN);
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    if (s < 0){
        perror("Could not create socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    rc = connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
   if (rc < 0){
        perror("Could not connect to server");
        exit(1);
    }

    fcntl(s, F_SETFL, O_NONBLOCK);
    pid_t pid;
    pid = fork();
    
    if (pid == 0){
        /*Child Process*/
        fh = fdopen(s, "r");
        char buf[BUFSIZE];
        do{
            /*Read one line from the client*/
            if (!fgets(buf, BUFSIZE, fh)){
                continue; /*connection was interrupted?*/
            }

            printf("%s",buf);
        } while (1); /*TODO close at some point*/
    }else{
        fh = fdopen(s, "w");
        do{
            printf(">");

            line = read_line();
            args = split_line(line, &argcount);
            state = execute(args, argcount);

            free(line);
            free(args);

        } while (state);
        fclose(fh);
        printf("Client Exit\n");
        kill(0, SIGKILL);
    }

    return 0;
}
