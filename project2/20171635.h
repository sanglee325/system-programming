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
#define MNEMONIC_LEN 10

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
	char mnemonic[MNEMONIC_LEN];
	bool format[5];
	struct _opcode_node *link;
} OPCODE_NODE;

typedef struct _symbol_table {
	int LOCCTR;
	char *symbol;
	struct _symbol_table *link;
} SYMBOL_TABLE;

typedef struct _optable {
	int opcode;
	int format;
	char mnemonic[MNEMONIC_LEN];
} OPTABLE;

typedef struct _symbol_set {
	char *symbol;
	char *mnemonic;
	char *operand;
} SYMBOL_SET;

typedef struct _flag_bit {
	int n;
	int i;
	int x;
	int b;
	int p;
	int e;
} FLAG_BIT;

typedef struct _register {
	int B;	// base register, used for addressing
	int S;	// general working register
	int T;	// general working register
	int F;	// floating point accumulator

	int A;	// Accumulator; used for arithmetic operations
	int X;	// index register; used for addressing
	int L;	// Linkage register; the jump to subroutine Instrution stores th return address here
	int PC;	// Program Counter
	int SW;	// status word; contains a variety of info including Condition Code
} REGISTER;

unsigned char *memory;
OPCODE_NODE *table[OPCODE_HASH_TABLE_SIZE];
SYMBOL_TABLE *symb_table[26];
REGISTER reg;
int format_num;

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
bool command_type(char *filename);
bool command_assemble(char *filename);

/*------ assembler -----*/
bool assemble_pass1(FILE* file_asm, int *program_len); 
bool isComment_check(const char* input);
void tokenize_input(char *input_asm, SYMBOL_SET *info, int *error);
int isLabel_check(const char *token0, const char *token1);
bool isOpcode_check(const char *token, int *format);
bool isDirective_check(const char *token);
void add_SYMBOL(SYMBOL_SET *info_input, int LOCCTR, int *error); 

void print_memory(int start, int end);
void character_print(int idx);
void read_opcode(FILE *fp);
void init_table();
void free_hash_table();
