#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_INPUT_LEN 100
#define MAX_CMD_LEN 10
#define	MEMORY_SIZE 1000000 

typedef struct _node {
	char str[MAX_INPUT_LEN];
	int num;
	struct _node *link;
} NODE;

typedef struct _historylist {
	NODE *head;
	NODE *tail;
} HISTORY_LIST;

unsigned char memory[MEMORY_SIZE];


void read_command(char *input_str); 

/*------ shell_command-----*/
void command_help();
void command_dir();
void command_quit();
void command_history();
void command_dump(int start, int end);
void command_edit();
void command_fill();
void command_reset();
void command_opcode_mnemonic();
void command_opcodelist();

void print_memory(int start, int end);
