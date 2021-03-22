#include "basic.h"
char* get_path(char* exe){
    char* path = NULL;
    if(exe[0] == '.' || exe[0]== '/') {
        return exe;
	}
    char* path_env = getenv("PATH");
    puts(path_env);
    char *token = strtok(path_env, ":");
    while(token != NULL) {
		path = strdup(token);
        strcat(path, "/");
        strcat(path, exe);
        if(access(path, X_OK) == 0) {
            return path;
        }
        token = strtok(NULL, ":");
        free(path);
    }
    return NULL;
}
int main(){
    char* path = get_path("sort");
    puts(path);
}

