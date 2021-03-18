#include "command.h"

char* substr(char* s, int start, int end) {
    char* subs = (char*) malloc(end - start);
    for(int i = start; i < end; ++i) {
        subs[i - start] = s[i];
    }

    return subs;
}

token_list* addCommand(token_list* list, char* c) {
    token_node* node = (token_node*) malloc(sizeof(token_node));
    node->token = (char*) malloc(sizeof(c));
    strcpy(node->token, c);
    node->next = NULL;

    if(list->size == 0) { // FIRST NODE
        list->head = node;
    } else {
        token_node* ptr = list->head;
        for(int i = 0; i < list->size - 1; ++i) {
            ptr = ptr->next;
        }
        ptr->next = node;
    }
    (list->size)++;
    return list;
}

void printList(token_list* list) {
    token_node* ptr = list->head;
    while(ptr != NULL) {
        printf("(%s)->", ptr->token);
        ptr = ptr->next;
    }
    printf("\n");
}

token_list* parseCommand(char* command) {
    token_list* list = (token_list*) malloc(sizeof(token_list));
    char* c;
    int start = 0;
    bool pipe = false;
    for(int i = 0;; ++i) {
        if(command[i] == '|' && command[i+1] != '|') { // SINGLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 1;
            list = addCommand(list, c);
            list = addCommand(list, "|");
        } else if(command[i] == '|' && command[i+1] == '|' && command[i+2] != '|') { // DOUBLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 2;
            list = addCommand(list, c);
            list = addCommand(list, "||");
            i++;
        } else if(command[i] == '|' && command[i+1] == '|' && command[i+2] == '|') { // TRIPLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 3;
            list = addCommand(list, c);
            list = addCommand(list, "|||");
            i += 2;
        } else if(command[i] == '\0') { // LAST COMMAND
            c = substr(command, start, i);
            list = addCommand(list, c);
            break;
        }
    }

    return list;
}