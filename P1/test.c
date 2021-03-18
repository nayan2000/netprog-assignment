#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __PROMPT__ 						"> "
#define __MAX_CMD_SIZE__ 			1024
#define __MAX_CMD_COUNT__ 			32
#define __MAX_OUT_SIZE__ 			1024
#define __MAX_FD_LIMIT__			1024
#define __MAX_SIMULT_CMDS__		1024
#define __PATH_ENV_VAR__ 			"PATH"
#define __CMD_DAEMONIZE__			"daemonize"
#define __CMD_FG__						"fg"
#define __CMD_BG__						"bg"
#define __CMD_JOBS__					"jobs"

#define __EMSG_UNHANDLED__		"Unhandled exception encountered. Exiting...\n"

#include<stdbool.h>
struct parsed_cmd {
    char ***cmd_opt_list;
    int dim;
};

typedef struct parsed_cmd PARSED_CMD;

char ** check_cmd (char *in_cmd, char *req_cmd) {
	char **args = malloc(2 * sizeof(char *));
	char *in_cmd_copy = strdup(in_cmd);

	char *token = strtok(in_cmd_copy, " ");

	if (token != NULL && strcmp(token, req_cmd) == 0) {
		if (token != NULL) {
			args[0] = strdup(token);
			token = strtok(NULL, " ");
		}

		if (token != NULL) {
			args[1] = strdup(in_cmd + strlen(args[0]) + 1);
		}
		else {
			args[1] = NULL;
		}

		free(in_cmd_copy);
		return args;
	}

	free(in_cmd_copy);
	args[0] = NULL;
	return args;
}

void FREE_CHECK_CMD_ARGS (char **args) {
	free(args);
}

PARSED_CMD* update_parsed_cmd (PARSED_CMD prev_parsed, char *delim) {
	PARSED_CMD* new_parsed = (PARSED_CMD *) malloc(sizeof(PARSED_CMD));
	new_parsed->dim = prev_parsed.dim;

	char ***new_cmd_opt = malloc(prev_parsed.dim * sizeof(char **));

	int i = 0;
	while (i < prev_parsed.dim) {
		int new_cmd_j = 0;
		new_cmd_opt[i] = malloc((__MAX_CMD_COUNT__ + 1) * sizeof(char *));

		int j = 0;
		while (prev_parsed.cmd_opt_list[i][j] != NULL) {
			char *cmd_copy = strdup(prev_parsed.cmd_opt_list[i][j]);
			char *token = strtok(cmd_copy, delim);
			if (token != NULL) {
				while (token != NULL) {
					new_cmd_opt[i][new_cmd_j] = token;
					new_cmd_j++;
					new_cmd_opt[i][new_cmd_j] = delim;
					new_cmd_j++;

					token = strtok(NULL, delim);
				}
				new_cmd_opt[i][new_cmd_j - 1] = NULL;
				new_cmd_j--;
			}
			j++;
		}

		i++;
	}
    int x = 0;
    printf("Old\n");
    while (x < prev_parsed.dim) {
		int i = 0;
		while (prev_parsed.cmd_opt_list[x][i] != NULL) {
			printf("as: %d %s\n", i, prev_parsed.cmd_opt_list[x][i]);
			i++;
		}
		x++;
	}
    printf("New\n");
    new_parsed->cmd_opt_list = new_cmd_opt;
    x = 0;
    while (x < new_parsed->dim) {
		int i = 0;
		while (new_parsed->cmd_opt_list[x][i] != NULL) {
			printf("as: %d %s\n", i, new_parsed->cmd_opt_list[x][i]);
			i++;
		}
		x++;
	}
    printf("\n");
	return new_parsed;
}

PARSED_CMD* parse_cmd (char *cmd) {
	PARSED_CMD init_p;
	init_p.dim = 1;
	init_p.cmd_opt_list = malloc(1 * sizeof(char **));
	init_p.cmd_opt_list[0] = malloc((__MAX_CMD_COUNT__ + 1) * sizeof(char *));

	init_p.cmd_opt_list[0][0] = cmd;
	init_p.cmd_opt_list[0][1] = NULL;

	PARSED_CMD* new_p = update_parsed_cmd(init_p, "|");
	new_p = update_parsed_cmd(*new_p, "#");
	new_p = update_parsed_cmd(*new_p, "S");
    int x = 0;
	// while (x < new_p->dim) {
	// 	int i = 0;
	// 	while (new_p->cmd_opt_list[x][i] != NULL) {
	// 		printf("as: %d %s\n", i, new_p->cmd_opt_list[x][i]);
	// 		i++;
	// 	}
	// 	x++;
	// }

	return new_p;


}

void FREE_CMD_LIST (char **cmd_list) {
	free(cmd_list);
}

int main () {
	parse_cmd("a|b#c|dSq");
	if(check_cmd("bg 123", "bg")) printf("asdfasdf\n");
}
