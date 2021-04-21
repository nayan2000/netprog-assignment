#include "basic.h"
#include "hashmap.c"

#define BUFLEN 512
#define PORT 33777

#define MAX_GROUP_LIMIT 10
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
    ip = malloc(16); 
    strcpy(ip, "239.0.0.");
    char l[3];
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
Use alarm signal for syncing files between group members
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
            char* mulip;
            printf("Enter name of group to create: ");
            readLine(in);
            genMulticastIP(mulip);
            insert(grpip, in, mulip);

            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(mulip);
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                printf("Error in creating group.\n");
                exit(1);
            }

        } else if(ch == 2) {
            // Set socket for broadcast
            int on = 1;
            int s_temp;
            if((s_temp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                printf("Error in creating broadcast socket.\n");
                exit(1);
            }

            setsockopt(s_temp, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

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

            if(sendto(s_temp, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending broadcast message.\n");
                exit(1);
            }

            // Wait for reply for sometime
            int retval;
            fd_set rfds;
            struct timeval tv;
            tv.tv_sec = BROADCAST_TIMEOUT;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(s, &rfds);
            retval = select(s_temp + 1, &rfds, NULL, NULL, &tv);

            if(retval == 0) {
                // Timeout happened
                printf("Couldn't find any groups with the given keyword.\n");
            } else {
                // TODO: Handle replies
            }


        } else if(ch == 3) {
            // Group list hashmap is displayed, user can chose which group to join from this list
            // To find more groups, search option can be used
            char* reqip;

            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(reqip);
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
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
            char* dest_ip = malloc(16);
            dest_ip = get(grpip, gname);

            struct sockaddr_in si_other;
            memset((char*) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(PORT);
            si_other.sin_addr.s_addr = inet_addr(dest_ip);

            if(sendto(s, msg, strlen(msg), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
                printf("Error in sending group message.\n");
                exit(1);
            }

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
    free_hm(grpip);
    return 0;

}