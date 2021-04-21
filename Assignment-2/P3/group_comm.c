#include "basic.h"
#include "hashmap.h"

#define BUFLEN 512
#define PORT 33777

#define MAX_GROUP_LIMIT 10

void readLine(char* msg) {
	char in = 'a';
	int i = 0;
	while(in != '\n') {
		in = getc(stdin);
		if(in != '\n') msg[i++] = in;
	}
}

char* genMulticastIP() {
    char* ip = malloc(16); 
    strcpy(ip, "239.0.0.");
    char l[3];
    int a = (rand() % 254) + 1;
    sprintf(l, "%d", a);
    strcat(ip, l);
    return ip;
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
USE ALARM SIGNAL FOR SYNCING FILES BETWEEN GROUP MEMBERS
*/

int main() {
    srand(time(0));

    // printf("%s\n", genMulticastIP());

	int s;
	char buf[BUFLEN];
	char message[BUFLEN];

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error in creating socket.\n");
        exit(1);
	}

    hashmap* grpip;
    create_hm(grpip, MAX_GROUP_LIMIT);

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
            printf("Enter name of group to create: ");
            readLine(in);
            insert(grpip, in, genMulticastIP());

        } else if(ch == 2) {
            // SET SOCKET FOR BROADCAST
            int on = 1;
            int s_temp;
            if((s_temp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                printf("Error in creating broadcast socket.\n");
                exit(1);
            }

            setsockopt(s_temp, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

            struct sockaddr_in si_other;
            char msg[10];
            if(sendto(s_temp, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending broadcast message.\n");
                exit(1);
            }


        } else if(ch == 3) {

        } else if(ch == 4) {

        } else if(ch == 5) {

        } else if(ch == 6) {

        } else if(ch == 7) {
            break;
        } else {
            printf("Invalid option, try again.\n");
        }
    }

    printf("Exiting...\n");
    close(s);
    return 0;

}