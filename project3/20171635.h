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
#define SYMBOL_HASH_TABLE_SIZE 26
#define MNEMONIC_LEN 10
#define OBJ_TEXT_RECORD 61
#define OBJ_CODE 61

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
	char symbol[20];
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

typedef struct _modification_record {
	int num_of_half_byte;
	int LOCCTR;
	struct _modification_record *link;
} MDR;

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

//unsigned char *memory;
unsigned char memory[MEMORY_SIZE];
OPCODE_NODE *table[OPCODE_HASH_TABLE_SIZE];
SYMBOL_TABLE *symb_table[SYMBOL_HASH_TABLE_SIZE];
bool flag_base;

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
bool command_symbol();

/*------ assembler -----*/
bool assemble_pass1(FILE* file_asm, int *program_len); 
bool assemble_pass2(int program_len, char *obj_file, char *list_file);
void tokenize_input(char *input_asm, SYMBOL_SET *info, int *error);
int isLabel_check(const char *token0, const char *token1);
bool isComment_check(const char* input);
bool isOpcode_check(const char *token, int *format, int *opcode);
bool isFormat_check(int format, const char *mnemonic, const char *operand);
bool isDirective_check(const char *token, char *symbol);
bool isEND_check(FILE *fp);
void add_SYMBOL(SYMBOL_SET *info_input, int LOCCTR, int *error); 
bool operand_directive(SYMBOL_SET *info_input, int *LOCCTR, int line_num); 
void tokenize_inter(char *input_asm, char *line_num, char *LOCCTR, char *format, char *label, char *mnemonic, char *operand, bool *comment);
int count_digits(int program_len);
void num_to_binary(int *opcode, int opcode_num, int size);
bool set_flagbit(FLAG_BIT *nixbpe, char *symbol, char *mnemonic, char *format, char *operand, int PC, int B, int *disp_add);
int search_symbol(const char *symbol);
void mnemonic_lst(FILE *lst, FILE *object, int *opcode, int *disp, FLAG_BIT nixbpe, char *line_num, char *LOCCTR, char *format, char *symbol, char *mnemonic, char *operand, int tot_digits, char *text_record, char *object_code);
void directive_lst(FILE *lst, FILE *object, char *line_num, char *LOCCTR, char *symbol, char *mnemonic, char *operand, int *disp, int tot_digits, char *text_record, char *object_code);
void create_obj(char *objcode, int* opcode, int *disp, FLAG_BIT nixbpe, char *format, char *mnemonic, char *operand);
char dec_to_hex(int dec);
char reg_to_num(char *reg);
void add_modification_record(MDR **mod_record, char *LOCCTR, int num_of_half_byte);

void print_memory(int start, int end);
void character_print(int idx);
void read_opcode(FILE *fp);
void init_table();
void init_symbol();
void free_hash_table();
