#include "job_table.h"
command_details* j_table[MAX_CMD];

bool add_entry(command_details* cmd_rec){
    int i = 0;
    while(i < MAX_CMD){
        if(!j_table[i]){
            j_table[i] = cmd_rec;
            return true;
        }
        i++;
    }
    return false;
}

bool remove_entry_by_id(int id){
    if(!j_table[id]) return false;
    free(j_table[id]->cmd);
    free(j_table[id]);
    j_table[id] = NULL;
    return true;
}

bool remove_entry_by_pgid(pid_t pgid){
    int id = pgid_to_id(pgid);
    if(id == -1) return false;
    free(j_table[id]->cmd);
    free(j_table[id]);
    j_table[id] = NULL;
    return true;
}

pid_t id_to_pgid(int id){
    if(!j_table[id]) return -1;
    return j_table[id]->pgid;
}

int pgid_to_id(pid_t pgid){
    int i = 0;
    while(i < MAX_CMD){
        if(j_table[i] && j_table[i]->pgid == pgid){
            return i;
        }
        i++;
    }
    return -1;
}

command_details* get_by_id(int id){
    if(!j_table[id]) return NULL;
    return j_table[id];
}

command_details* get_by_pgid(pid_t pgid){
    int id = pgid_to_id(pgid);
    if(!j_table[id]) return NULL;
    return j_table[id];
}


bool update_entry_by_pgid(pid_t pgid, j_type type, j_stat status){
    command_details* cmd_rec = get_by_pgid(pgid);

    if(cmd_rec != NULL){
        cmd_rec->type = type;
        cmd_rec->status = status;
        return true;
    }
    return false;
}