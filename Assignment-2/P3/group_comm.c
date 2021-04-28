#include "basic.h"
#include "hashmap.c"

#define BUFLEN 512
#define PORT 33777
#define BROADCAST_PORT 33333

#define MAX_GROUP_LIMIT 10
#define MAX_FILE_LIMIT 30
#define BROADCAST_TIMEOUT 1

void readLine(char* msg) {
	char in = 'a';
	int i = 0;
	while(in != '\n') {
		in = getc(stdin);
		if(in != '\n') msg[i++] = in;
	}
}

void genMulticastIP(char* ip) {
    ip = (char*)malloc(16); 
    strcpy(ip, "239.0.0.");
    char l[4] = {0};
    int a = (rand() % 254) + 1;
    sprintf(l, "%d", a);
    strcat(ip, l);
}

void sendGrpMsg(char* dest_ip, char msg[BUFLEN]) {
    struct sockaddr_in si_dest;
    int slen = sizeof(si_dest);
	memset((char*) &si_dest, 0, slen);
	si_dest.sin_family = AF_INET;
	si_dest.sin_port = htons(PORT);
	si_dest.sin_addr.s_addr = inet_addr(dest_ip);
}

/*
    Message Formats:
    B - Broadcast Message
    G - Group Message
    P - Poll Message
    F - Filelist Request Message
    R - Filelist Reply Message
*/


/*
Use alarm signal for syncing files between group members
*/
hashmap *grpip, *filelist;
int sockfd_m;

void syncfiles() {
    for(int i = 0; i < grpip->capacity; ++i) {
        bucket_node* temp = grpip->buckets[i];
        while(temp != NULL) {
            char* dest_ip = (char*)malloc(16);
            dest_ip = get(grpip, temp->key);

            char msg[2] = "F:";

            struct sockaddr_in si_other;
            memset((char*) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(PORT);
            si_other.sin_addr.s_addr = inet_addr(dest_ip);

            if(sendto(sockfd_m, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending group message.\n");
                exit(1);
            }
            temp = temp->next;
        }
    }
    alarm(60);
}

typedef struct pthread_args{
    int sockfd_m;
    int sockfd_b;
    hashmap* grpip;
    hashmap* filelist;
}pthread_args;

// Running on separate thread
void handleMessages(void* arg) {
    pthread_args* args = (pthread_args*)arg;
    int sockfd_m = args->sockfd_m;
    int sockfd_b = args->sockfd_b;
    hashmap* grpip = args->grpip;
    hashmap* filelist = args->filelist;
    struct sockaddr_in clAddr;
    socklen_t len = sizeof(struct sockaddr_in);
    bzero(&clAddr, sizeof(struct sockaddr_in));
    while(1){
        char recvbuf[BUFSIZ] = {0};
        int retval;
        fd_set rfds;
        // struct timeval tv;
        // tv.tv_sec = BROADCAST_TIMEOUT;
        // tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(sockfd_m, &rfds);
        int maxfd = sockfd_m;
        retval = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if(retval == -1 && errno == EINTR) continue;

        if(FD_ISSET(sockfd_m, &rfds)){
            len = sizeof(struct sockaddr_in);
            recvfrom(sockfd_m, recvbuf, BUFSIZ, 0, (struct sockaddr*) &clAddr, &len);
            if(recvbuf[0] == 'B' && recvbuf[1] == ':'){
                char sendbuf[50] = {0};
                strcpy(sendbuf, recvbuf + 2);
                group* g = get(grpip, sendbuf);
                char ip[16] = {0};
                strcpy(ip, g->val);
                strcat(sendbuf, ":");
                strcat(sendbuf, ip);
                strcat(sendbuf, "\n");
                if(sendto(sockfd_b, sendbuf, strlen(sendbuf), 0, (struct sockaddr*) &clAddr, len) == -1) {
                    printf("Error in sending broadcast message.\n");
                    exit(1);
                }
            }
            else if(recvbuf[0] == 'G' && recvbuf[1] == ':'){
                char* addr[20] = {0};
                inet_ntop(AF_INET, clAddr.sin_addr.s_addr, addr, 20);
                bucket_node *temp;
                for(int i = 0; i < MAX_GROUP_LIMIT; i++){
                    if(grpip->buckets[i]){
                        temp = grpip->buckets[i];
                        while(temp != NULL){
                            if(!strcmp((temp->val).val, addr)){
                                break;
                            }
                            temp = temp->next;
                        }
                    }
                }
                printf("Message recieved for group : %s\n", temp->key);
                printf("%s", recvbuf + 2);
            } else if(recvbuf[0] == 'F' && recvbuf[1] == ":") {
                char msg[500] = "R:";

                /* 
                    Message Format:
                    R:client_ip:file1:file2:file3:...
                */

                struct sockaddr_in ip;
                socklen_t iplen = sizeof(ip);
                char cl_ip[16] = {0};
                getsockname(sockfd_m, (struct sockaddr*)&ip, &iplen);
                inet_ntop(AF_INET, &(ip.sin_addr), cl_ip, INET_ADDRSTRLEN);
                strcat(msg, cl_ip);
                strcat(msg, ":");
                for(int i = 0; i < filelist->capacity; ++i) {
                    bucket_node* temp = filelist->buckets[i];
                    while(temp != NULL) {
                        strcat(msg, temp->key);
                        strcat(msg, ":");
                        temp = temp->next;
                    }
                }
                if(sendto(sockfd_m, msg, strlen(msg), 0, (struct sockaddr*) &clAddr, len) == -1) {
                    printf("Error in sending filelist reply message.\n");
                }
            } else if(recvbuf[0] == 'R' && recvbuf[1] == ':') {
                char cl_ip[16] = {0};
                strcpy(cl_ip, recvbuf + 2);
                char* token = strtok(recvbuf, ":");
                strcpy(cl_ip, strtok(NULL, ":"));
                token = strtok(NULL, ":");
                while(token) {
                    insert2(filelist, token, cl_ip);
                    token = strtok(NULL, ":");
                }
            }
        }
        
    }
}

int main() {
    signal(SIGALRM, syncfiles);
    srand(time(0));
	char buf[BUFLEN] = {0};
	char message[BUFLEN] = {0};

	if((sockfd_m = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error in creating multicast socket.\n");
        exit(1);
	}
// Set socket for broadcast
    int on = 1;
    int sockfd_b;
    if((sockfd_b = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("Error in creating broadcast socket.\n");
        exit(1);
    }

    setsockopt(sockfd_b, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    grpip = (hashmap*)malloc(sizeof(hashmap));
    filelist = (hashmap*)malloc(sizeof(hashmap));
    create_hm(grpip, MAX_GROUP_LIMIT);
    create_hm(filelist, MAX_FILE_LIMIT);


    // printf("%s\n", genMulticastIP());
    pthread_t thread_id;
    pthread_args args = {sockfd_m, sockfd_b, grpip, filelist};
    pthread_create(&thread_id, NULL, handleMessages, (void*)&args);

	
    int b;
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    if((b = bind(sockfd_m, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in))) < 0){
        perror(RED"BIND ERROR"RESET);
        exit(EXIT_FAILURE);
    }

    

    int ch = 0;
    while(1) {
        printf("------------------------\n");
        printf("          MENU          \n");
        printf("------------------------\n");
        printf("1. Create a group\n");
        printf("2. Search for a group\n");
        printf("3. Join a group\n");
        printf("4. Send a group message\n");
        printf("5. Download a file\n");
        printf("6. Start a group poll\n");
        printf("7. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &ch);

        if(ch == 1) {
            char in[30];
            char* mulip;
            printf("Enter name of group to create: ");
            readLine(in);
            genMulticastIP(mulip);
            group g;
            strcpy(g.val, mulip);
            g.is_member = true;
            insert(grpip, in, g);

            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(mulip);
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if(setsockopt(sockfd_m, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                printf("Error in creating group..\n");
                exit(1);
            }
            printf("Group created..\n");

        }else if(ch == 2) {
            struct sockaddr_in si_other;
            memset((char*) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(PORT);
            si_other.sin_addr.s_addr = inet_addr("255.255.255.255");
            /*
                Message Format:
                B:keyword
            */

            char keyw[20], msg[22];
            strcpy(msg, "B:");

            printf("Enter keyword to search for groups: ");
            scanf("%s", keyw);
            strcat(msg, keyw);

            if(sendto(sockfd_m, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending broadcast message.\n");
                exit(1);
            }
            char recvbuf[BUFSIZ] = {0};
            struct sockaddr_in clAddr;

            socklen_t len = sizeof(struct sockaddr_in);
            bzero(&clAddr, sizeof(struct sockaddr_in));
            int retval;
            fd_set rfds;
            for(;;){
                struct timeval tv;
                tv.tv_sec = BROADCAST_TIMEOUT;
                tv.tv_usec = 0;
                FD_ZERO(&rfds);
                FD_SET(sockfd_b, &rfds);
                int maxfd = sockfd_b;
                retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
                if(retval == -1 && errno == EINTR) continue;
                if(retval == 0) break;
                recvfrom(sockfd_b, recvbuf, BUFSIZ, 0, &clAddr, len);

                char key[20] = {0};
                int i;
                for(i = 0; recvbuf[i] != ':' && recvbuf[i]; i++){
                    key[i] = recvbuf[i];
                }
                group g;
                bzero(&g, sizeof(g));
                i++;
                for(; recvbuf[i]; i++){
                    g.val[i] = recvbuf[i];
                }
                g.is_member = false;
                if(!has_key(grpip, key)){
                    insert(grpip, key, g);
                }
                printf(YELLOW"Group : %s , Group IP : %s\n"RESET, key, g.val[i]);
            }

        }else if(ch == 3){
            // Group list hashmap is displayed, user can chose which group to join from this list
            // To find more groups, search option can be used
            char gname[20] = {0};
            printf("Enter group name to join : ");
            readLine(gname);
            group* g = get(grpip, gname);
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(g->val);
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if(setsockopt(sockfd_m, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                printf("Error in joining group.\n");
                exit(1);
            }

            printf("Successfully joined group.\n");

        } else if(ch == 4) {
            /*
                Message Format:
                G:message
            */

            char gname[20], txtmsg[30], msg[32] = "G:";
            printf("Enter group name to send message to: ");
            readLine(gname);
            printf("Enter the message: ");
            readLine(txtmsg);
            strcat(msg, txtmsg);
            char* dest_ip = (char*)malloc(16);
            dest_ip = get(grpip, gname);

            struct sockaddr_in si_other;
            memset((char*) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(PORT);
            si_other.sin_addr.s_addr = inet_addr(dest_ip);

            if(sendto(sockfd_m, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending group message.\n");
                exit(1);
            }

        } else if(ch == 5) {
            // Search for filename in local list first, if file is not in list, send special message to all group members of groups the user belongs to
            char fname[20];
            printf("Enter file name to download: ");
            readLine(fname);

            if(has_key(filelist, fname)) {
                // File is present in local list
            } else {
                // Send special message
            }

        } else if(ch == 6) {
            char gname[20], pmsg[20];
            char** options;
            int i = 0;
            options[i] = (char*)malloc(20);
            printf("Enter group to send poll message to: ");
            readLine(gname);
            printf("Enter poll question: ");
            readLine(pmsg);
            char in[20];
            while(1) {
                printf("Enter poll option(%d): ", i + 1);
                readLine(in);
                strcpy(options[i], in);
                printf("Add more options? (Y/N): ");
                readLine(in);
                if(!strcmp(in, "Y")) {
                    options[++i] = (char*)malloc(20);
                } else break;
            }

            /*
                Message Format:
                P:poll_question:poll_option_1:poll_option_2...
            */

            char msg[300] = "P:";
            strcat(msg, pmsg);
            for(int j = 0; j < i + 1; ++j) {
                strcat(msg, options[j]);
            }

            printf("%s\n", msg);

            char* dest_ip = (char*)malloc(16);
            dest_ip = get(grpip, gname);

            struct sockaddr_in si_other;
            memset((char*) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(PORT);
            si_other.sin_addr.s_addr = inet_addr(dest_ip);

            if(sendto(sockfd_m, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending group poll.\n");
                exit(1);
            }

            // Handle replies

        } else if(ch == 7) {
            break;
        } else {
            printf("Invalid option, try again.\n");
        }
    }

    printf("Exiting...\n");
    close(sockfd_m);
    free_hm(grpip);
    return 0;

}