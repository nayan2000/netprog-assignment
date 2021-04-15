#include "parse_input.h"

void trim(char *s, bool space) {
    char *p = s;
    int l = strlen(p);
    if(space){
        while(isspace(p[l - 1])) p[--l] = 0;
        while(*p && isspace(* p)) ++p, --l;
    }
    else{
        while(p[l - 1] == '"') p[--l] = 0;
        while(*p && *p == '"') ++p, --l;
    }

    memmove(s, p, l + 1);
}

char* substr(char* s, int start, int end) {
    char* subs = (char*) malloc(sizeof(char)*(end - start + 1));
    int i;
    for(i = start; i < end; ++i) {
        subs[i - start] = s[i];
    }
    subs[i-start] = '\0';
    return subs;
}

cmd_list* parse_inp(char* inp){
    cmd_list *list = (cmd_list*)malloc(sizeof(cmd_list));
    list->size = 0;
    char* inp_cpy = strdup(inp);
    char *token = strtok(inp_cpy, "|");
    cmd_node *temp = NULL;
    char **args = (char**)malloc(sizeof(char*)*2);

    while(token != NULL){
        int l = 0, r = 0;
        temp = (cmd_node*)malloc(sizeof(cmd_node));
        char* tk = strdup(token);
        while(tk[r] && tk[r] != '.'){
            r++;
        }
        if(tk[r] == 0)
            return NULL;
        args[0] = substr(tk, l, r);
        l = r + 1;
        while(tk[r]){
            r++;
        }
        args[1] = substr(tk, l, r);
        trim(args[0], true);
        trim(args[1], true);
        temp->node = args[0];
        temp->command = args[1];
        if(list->size == 0){
            list->head = temp;
        }else{
            cmd_node* ptr = list->head;
            for(int i = 0; i < list->size - 1; ++i)
                ptr = ptr->next;
            ptr->next = temp;
        }
        free(tk);
        token = strtok(NULL, "|");
        (list->size)++;
    }
    free(inp_cpy);
    return list;
}

void remove_list(cmd_list *list){
    cmd_node* temp = list->head;
    cmd_node* next;
    while(temp != NULL){
        next = temp->next;
        free(temp->command);
        free(temp->node);
        free(temp);
        temp = next;
    }
    free(list);
}

void print_list(cmd_list* list) {
    if(list == NULL){
        return;
    }
    cmd_node* ptr = list->head;
    while(ptr->next != NULL) {
        printf("(%s %s)->", ptr->node, ptr->command);
        ptr = ptr->next;
    }
    printf("(%s %s)", ptr->node, ptr->command);
    printf("\n");
}

// int main(){
//     char command[] = "n1.ls-l";
//     cmd_list* list = parse_inp(command);
//     print_list(list);
//     char x[20] = {0};
//     printf("%d", strlen(x));
//     // remove_list(list);
//     // print_list(list);
// }

