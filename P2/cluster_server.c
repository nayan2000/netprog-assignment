#include "basic.h"
#include "setup_utilities.h"
#include "get_config.h"
#include "parse_input.h"
#include "inet_sockets.h"
#define MAX_BUF_SZ 2048
#define MAX_OUTPUT 2048
#define MAX_INPUT 2048

char **client_ips = NULL;

int is_alive(char *ipaddr){
	int sockfd;
	struct sockaddr_in serveraddr;
	if((sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
			perror("Socket: Open");
	serveraddr.sin_port = htons(atoi(CLIENT_PORT));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
	if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
		close(sockfd);
		return 0;
	}
	else{
		close(sockfd);
		return 1;
	}
}

bool handle_nodes_cmd(char* cmd, char* buf){
	if(strcmp(cmd, "nodes") == 0) {
		for(int i = 1; i < MAX_NODES+1; i++){
			char *ip = client_ips[i];
			if(is_alive(ip) == 1){
				sprintf(buf, GREEN"n[%d] - %s : Online\n"RESET, i, ip);
			}
			else{
				sprintf(buf, RED"n[%d] - %s : Offline\n"RESET, i, ip);
			}
		}
		return true;
	}
	return false;
}
void handle_request(int cfd, char * address_string){
	int n;
	char cmd[MAX_BUF_SZ] = {0};
	while(n = read(cfd, cmd, MAX_BUF_SZ) > 0){
		cmd[n] = 0;
		printf(GREEN"%d - Command: %s\n\tClient : %s\n"RESET, n, cmd, address_string);
		
		char input_buf[3*MAX_INPUT + 1] = {0};
		int inp_sz = 0;
		if(handle_nodes_cmd(cmd, input_buf)){
			write(cfd, input_buf, strlen(input_buf) + 1);
			continue;
		}

		cmd_list* list = parse_inp(cmd);
		cmd_node* temp = list->head;


		while(temp != NULL){
			char* node_id = temp->node;
			char* command = temp->command;
			if(command[1] == '*'){
				char combined[3*MAX_OUTPUT+1] = {0};

				for(int i = 1; i < MAX_NODES+1; i++){
					char *ip = client_ips[i];
					int connfd = inet_connect(ip, CLIENT_PORT, SOCK_STREAM);
					if(connfd < 0) continue;		
					char send_buf[MAX_BUF_SZ] = {0};
					strcpy(send_buf, command);
					if(strlen(input_buf))
						strcpy(send_buf+strlen(command)+1, input_buf);
					int len = strlen(command) + strlen(input_buf) + 2;
					int nbytes = write(connfd, send_buf, len);
					if(nbytes != len) {
						perror(RED"WRITE"RESET);
					}
					char recv_buf[MAX_OUTPUT + 1];
					int n = read(connfd, recv_buf, MAX_OUTPUT+1);
					recv_buf[n] = 0;
					strcat(combined, recv_buf);
					close(connfd);
				}
				inp_sz = strlen(combined)+1;
				strcpy(input_buf, combined);
				
			}
			else{
				int i = atoi(node_id + 1);
				char *ip = client_ips[i];
				int connfd = inet_connect(ip, CLIENT_PORT, SOCK_STREAM);
				if(connfd < 0) continue;		
				char send_buf[MAX_BUF_SZ] = {0};
				strcpy(send_buf, command);
				if(strlen(input_buf))
					strcpy(send_buf+strlen(command)+1, input_buf);
				int len = strlen(command) + strlen(input_buf) + 2;
				int nbytes = write(connfd, send_buf, len);
				if(nbytes != len) {
					perror(RED"WRITE"RESET);
				}
				inp_sz = read(connfd, input_buf, MAX_OUTPUT+1);
				input_buf[n] = 0;
				close(connfd);
			}
		}
		write(cfd, input_buf, inp_sz);
	}
	free(address_string);
	close(cfd);
}


int main(int argc, char *argv[])
{
    client_ips = load_config_file("config");
    int lfd = inet_listen(SERV_PORT, BACKLOG, NULL);
	printf(YELLOW"SERVER STARTED\n"RESET);
	
    struct sockaddr_in claddr;
    int cfd;
    socklen_t addrlen;
	for(;;) {
		addrlen = sizeof(claddr);
		if ((cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen)) < 0) {
			if (errno == EINTR) continue; /* back to for() */ 
			else{
            	fprintf(stderr, RED"ERROR : CANT'T ACCEPT ON A CLIENT\n"RESET);
				exit(0);
			}
		
		}
		char* address_string = (char*)malloc(sizeof(char)*IS_ADDR_STR_LEN);

		inet_address_str((struct sockaddr*)&claddr, addrlen, address_string, IS_ADDR_STR_LEN);
		printf(GREEN"Handling client %s\n"RESET, address_string);

		pid_t child = fork();
		switch(child){
			case -1:
				printf(RED"ERROR: CHILDREN CAN'T BE CREATED\n"RESET);	
				exit(EXIT_FAILURE);

			case 0:
				close(lfd);	
				handle_request(cfd, address_string);			
				_exit(EXIT_SUCCESS);
			default:
				close(cfd);
				break;
		}
    }
}