#include "parse_command.h"

char* substr(char* s, int start, int end) {
    char* subs = (char*) malloc(sizeof(char)*(end - start + 1));
    int i;
    for(i = start; i < end; ++i) {
        subs[i - start] = s[i];
    }
    subs[i-start] = '\0';
    return subs;
}

token_list* add_command(token_list* list, char* c) {
    token_node* node = (token_node*) malloc(sizeof(token_node));
    // TRIM WHITE SPACE FROM SIDES IN COMMAND
    int status = 0, l_pos, r_pos, com_size;
    for(int i = 0;; ++i) {
        if(status == 0 && c[i] != ' ') { // FOUND FIRST LEFT CHAR
            l_pos = i;
            status = 1;
        }
        if(status == 1 && c[i] == '\0') { // END OF STRING
            r_pos = i - 1;
            while(c[r_pos] == ' ') { // GO BACK TO FIND LAST RIGHT CHAR
                r_pos--;
            }
            break;
        }
    }

    com_size = r_pos - l_pos + 1;
    node->token = (char*) malloc(sizeof(char)*(com_size+1));
    int i;
    for(i = l_pos; i <= r_pos; ++i) {
        (node->token)[i - l_pos] = c[i];
    }
    (node->token)[i - l_pos] = '\0';
    node->next = NULL;

    if(list->size == 0){ // FIRST NODE
        list->head = node;
    }else{
        token_node* ptr = list->head;
        for(int i = 0; i < list->size - 1; ++i){
            ptr = ptr->next;
        }
        ptr->next = node;
    }
    (list->size)++;
    return list;
}

void print_list(token_list* list) {
    token_node* ptr = list->head;
    while(ptr != NULL) {
        printf("(%s)->", ptr->token);
        ptr = ptr->next;
    }
    printf("\n");
}
void free_list(token_list* list){
    token_node* ptr = list->head;
    token_node* next = NULL;
    while(ptr){
        next = ptr->next;
        free(ptr->token);
        free(ptr);
        ptr = next;
    }
    free(list);
}

token_list* parse_cmd(char* command){
    token_list* list = (token_list*) malloc(sizeof(token_list));
    char* c;
    int start = 0;
    bool pipe = false;
    for(int i = 0;; ++i) {
        if(command[i] == '|' && command[i+1] != '|') { // SINGLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 1;
            list = add_command(list, c);
            list = add_command(list, "|");
        }else if(command[i] == '|' && command[i+1] == '|' && command[i+2] != '|') { // DOUBLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 2;
            list = add_command(list, c);
            list = add_command(list, "||");
            i++;
        }else if(command[i] == '|' && command[i+1] == '|' && command[i+2] == '|') { // TRIPLE PIPE
            pipe = true;
            c = substr(command, start, i);
            start = i + 3;
            list = add_command(list, c);
            list = add_command(list, "|||");
            i += 2;
        }else if(command[i] == '\0') { // LAST COMMAND
            c = substr(command, start, i);
            list = add_command(list, c);
            break;
        }
    }
    return list;
}
