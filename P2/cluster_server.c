#include "basic.h"
#include "setup_utilities.h"
#include "get_config.h"

int main(int argc, char *argv[])
{
    char **client_ips = load_config_file("config");
    int lfd = server_setup(SERV_PORT);
	printf(YELLOW"SERVER STARTED\n"RESET);
    struct sockaddr_in claddr;
    int cfd;
    socklen_t addrlen;


    while(1) {
		addrlen = sizeof(claddr);
		if ((cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen)) < 0) {
            fprintf(stderr, RED"ERROR : CANT'T ACCEPT ON A CLIENT\n"RESET);
			continue;
		}

		printf(GREEN"Handling client %s\n"RESET, inet_ntoa(claddr.sin_addr));

		pid_t child = fork();

		if(child < 0) {
			printf("Error handling the forking. Exiting...\n");	
			exit(0);
		}

		else if(child == 0) {
			close(lfd);

        }
    }
}