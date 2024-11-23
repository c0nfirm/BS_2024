#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void sig_handler(int signo) {
	const char msg[] = "Speicherzugriffsfehler!\n";
	write(2, msg, sizeof(msg) - 1);
	exit(-1);
}

int main(void) {
	if (signal(SIGSEGV, sig_handler) == SIG_ERR) {
		perror("signal");
		exit(-1);
	}
	
	int array[1] = {0};
	array[1000000] = 3;
	return 0;
}
