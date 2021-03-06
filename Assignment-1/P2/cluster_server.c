#include "cluster_server.h"
char **client_ips = NULL;

bool handle_nodes_cmd(char* cmd, char* buf){
	if(strcmp(cmd, "nodes") == 0) {
		char suffix[50] = {0};
		for(int i = 1; i < MAX_NODES+1; i++){
			char *ip = client_ips[i];
			if(ip){
				sprintf(suffix, GREEN"n[%d] - %s : Online\t"RESET, i, ip);
				strcat(buf, suffix);
			}
		}
		strcat(buf, "$");
		return true;
	}
	return false;
}
void handle_request(int cfd, char * address_string){
	ssize_t nread = 0;
	while(1){
		jmp:;
		char cmd[MAX_BUF_SZ] = {0};
		read_buf reader;
		bzero(&reader, sizeof(reader));
		readline_buf_init(cfd, &reader);
		nread = readline_buf(&reader, cmd, MAX_BUF_SZ);
		if(nread <= 0) break;
		if(cmd[nread - 1] == '$')
			cmd[nread-1] = 0; 		/*Remove $*/
		char input_buf[3*MAX_INPUT + 1] = {0};
		
		int inp_sz = 0;
		if(handle_nodes_cmd(cmd, input_buf)){
			write(cfd, input_buf, strlen(input_buf) + 1);
			continue;
		}

		cmd_list* list = parse_inp(cmd);
		if(list == NULL){
			char input[20] = "Input is invalid\n$";
			write(cfd, input, strlen(input) + 1);
		}
		else{
			print_list(list);
			cmd_node* temp = list->head;
			input_buf[0] = '$';
			while(temp != NULL){
				char* node_id = temp->node;
				char* command = temp->command;
				if(node_id[1] == '*'){
					char combined[3*MAX_OUTPUT+1] = {0};

					for(int i = 1; i < MAX_NODES+1; i++){
						if(client_ips[i]){							
							char *ip = client_ips[i];
							printf("Node IP : %s\n", ip);
							/* For testing purposes */
							int connfd;
							if(i == 1)
								connfd = inet_connect(ip, CLIENT_PORT1, SOCK_STREAM);
							else
								connfd = inet_connect(ip, CLIENT_PORT2, SOCK_STREAM);
							/* For non-testing purposes */
							/* int connfd;
							connfd = inet_connect(ip, CLIENT_PORT, SOCK_STREAM); */
							if(connfd < 0){
								perror(RED"Connect error while executing child command"RESET);
								char temp[40] = {0};
								sprintf(temp, "Connect error for node %d, IP %s\n$", i, ip);
								write(cfd, temp, strlen(temp) + 1);
								goto jmp;
							}

							char send_buf[MAX_BUF_SZ] = {0};
							strcpy(send_buf, command);
							
							if(strlen(input_buf))
								strcpy(send_buf+strlen(command)+1, input_buf);
							
							int len = strlen(command) + strlen(input_buf) + 2;
							int nbytes = write(connfd, send_buf, len);
							if(nbytes != len) {
								perror(RED"WRITE"RESET);
							}

							read_buf out_reader;
							bzero(&out_reader, sizeof(out_reader));
							readline_buf_init(connfd, &out_reader);

							char recv_buf[MAX_OUTPUT + 1] = {0};
							int n = readline_buf(&out_reader, recv_buf, MAX_OUTPUT+1);
							recv_buf[n-1] = '\t';
							strcat(combined, recv_buf);
							close(connfd);
						}
					}
					strcat(combined, "$");
					inp_sz = strlen(combined) + 1;
					strcpy(input_buf, combined);

					puts(input_buf);
				}
				else{

					printf("Node ID : %s, ", node_id);
					int i = atoi(node_id + 1);
					printf("Value : %d\n", i);
					char *ip = client_ips[i];
					printf("Node IP : %s\n", ip);
					/* For testing purposes */
					int connfd;
					if(i == 1)
						connfd = inet_connect(ip, CLIENT_PORT1, SOCK_STREAM);
					else
						connfd = inet_connect(ip, CLIENT_PORT2, SOCK_STREAM);
					/* For non-testing purposes */
					/* int connfd;
					connfd = inet_connect(ip, CLIENT_PORT, SOCK_STREAM); */
					if(connfd < 0){
						perror(RED"Connect error while executing child command"RESET);
						char temp[40] = {0};
						sprintf(temp, "Connect error for node: %d, IP: %s\n$", i, ip);
						write(cfd, temp, strlen(temp) + 1);
						goto jmp;
					}
					
					char send_buf[MAX_BUF_SZ] = {0};
					strcpy(send_buf, command);
					if(strlen(input_buf))
						strcpy(send_buf+strlen(command)+1, input_buf);
					
					int len = strlen(command) + strlen(input_buf) + 2;
					/* command-0-input_buf$-0*/
					int nbytes = write(connfd, send_buf, len);

					if(nbytes != len) {
						perror(RED"WRITE"RESET);
					}
					read_buf out_reader;
					bzero(&out_reader, sizeof(out_reader));
					readline_buf_init(connfd, &out_reader);

					bzero(input_buf, strlen(input_buf)+1);
					inp_sz = readline_buf(&out_reader, input_buf, MAX_OUTPUT+1);
					/* input_buf$ */
					input_buf[inp_sz] = 0;
					inp_sz++;
					close(connfd);
				}
				temp = temp->next;
			}
			write(cfd, input_buf, inp_sz);
		}
	}
	printf(GREEN"Client Exiting: %s\n"RESET, address_string);
	free(address_string);

	close(cfd);
}


int main(int argc, char *argv[]){
    client_ips = load_config_file("config");
    int lfd = inet_listen(SERV_PORT, BACKLOG, NULL);
	if(lfd == -1){
		perror(RED"Listen error"RESET);
		exit(EXIT_FAILURE);
	}
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
		fflush(stdout);
		fflush(stdin);
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