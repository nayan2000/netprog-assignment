/*Header Guard*/
#ifndef SC_TABLE_H
#define SC_TABLE_H

#include "basic.h"
#include "parse_command.h"
#define MAX_SC_TABLE_SIZE 40

void print_sc_table();
bool manage_sc_command(char** broken_cmd, token_list* list);
bool remove_from_sc_table(int i);
char* lookup_cmd(int i);
bool add_to_sc_table(int i, char* cmd);

#endif