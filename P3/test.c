#include "basic.h"
#include <stddef.h>
#define PATH_MAX 30
struct requestMsg { 
    long mtype; 
    int clientId; 
    char pathname[PATH_MAX];
};
#define REQ_MSG_SIZE (offsetof(struct requestMsg, pathname) - offsetof(struct requestMsg, clientId) + PATH_MAX)

int main(){
    char command[40] = {0};
    printf("%ld\n", sizeof(command));
    struct requestMsg ms;
    
    printf("%d, %d, %d, %d, %ld\n", sizeof(struct requestMsg), sizeof(struct requestMsg) - sizeof(long), offsetof(struct requestMsg, pathname), offsetof(struct requestMsg, clientId), REQ_MSG_SIZE);
}