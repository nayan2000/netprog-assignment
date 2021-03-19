#ifndef COMMAND_TABLE_H
#define COMMAND_TABLE_H

#include "basic.h"

#define MAX_CMD 40

typedef enum j_type{
    BG, FG
}j_type;

typedef enum j_stat{
    RUN, STOP 
}j_stat;

typedef struct command_details{
    char* cmd;
    pid_t pgid;
    j_type type;
    j_stat status;
}command_details;

bool add_entry(command_details* cmd_rec);
bool remove_entry_by_id(int id);
bool remove_entry_by_pgid(pid_t pgid);
pid_t id_to_pgid(int id);
int pgid_to_id(pid_t pgid);
command_details* get_by_id(int id);
command_details* get_by_pgid(pid_t pgid);
bool update_entry_by_pgid(pid_t pgid, j_type type, j_stat status);

#endif