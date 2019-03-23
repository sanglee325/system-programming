#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_INPUT_LEN 500
#define MAX_CMD_LEN 10
#define	MEMORY_SIZE 0x100000
#define OPCODE_HASH_TABLE_SIZE 20
#define OPCODE_MNEMONIC_LEN 10

typedef struct _history_node {
	char str[MAX_INPUT_LEN];
	int num;
	struct _history_node *link;
} HISTORY_NODE;

typedef struct _historylist {
	HISTORY_NODE *head;
	HISTORY_NODE *tail;
} HISTORY_LIST;

typedef struct _opcode_node {
	int opcode;
	char mnemonic[OPCODE_MNEMONIC_LEN];
	bool format[5];
	struct _opcode_node *link;
} OPCODE_NODE;

unsigned char *memory;
OPCODE_NODE *table[OPCODE_HASH_TABLE_SIZE];


void read_command(char *input_str); 

/*------ shell_command-----*/
void command_help();
void command_dir();
void command_quit();
void command_history();
void command_dump(int start, int end);
void command_edit(int address, int value);
void command_fill(int start, int end, int value);
void command_reset();
bool command_opcode(char *mnemonic);
void command_opcodelist();

void print_memory(int start, int end);
void character_print(int idx);
void read_opcode(FILE *fp);
void init_table();
void free_hash_table();
