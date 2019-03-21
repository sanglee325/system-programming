#include "20171635.h"

extern bool exit_flag;
extern HISTORY_LIST *history;

/***** function for command help *****/
void command_help() {
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
}

/***** function for command dir *****/
void command_dir() {
	DIR *dp = NULL;
	struct dirent *entry = NULL;
	struct stat buf;

	dp = opendir(".");
	if(dp == NULL){
		printf("ERROR: directory open failure\n");
		return;
	}

	entry = readdir(dp);
	while((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &buf);

		//directory type file
		if(S_ISDIR(buf.st_mode)) {
			printf("%s/  ", entry->d_name);
		}
		else if(S_ISREG(buf.st_mode)) {
			//executable file
			if((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXOTH)) {
				printf("%s*  ", entry->d_name);
			}
			//regular file
			else {
				printf("%s  ", entry->d_name);
			}
		}
	}
	printf("\n");

	closedir(dp);
}

/***** function for command quit *****/
void command_quit() {
	HISTORY_NODE *target, *temp;

	exit_flag = true;
	if(history != NULL) {
		target = history->head;
		temp = target->link; 

		while(1) {
			temp = target->link;
			free(target);
			if(target->link == NULL) break;
			target = temp;
		}
	}
}

/***** function for command history *****/
void command_history(){
	HISTORY_NODE *target;

	target = history->head;
	while(1){
		printf("%-5d%s", target->num, target->str);
		if(target->link == NULL) {
			break;
		}
		else {
			target = target->link;
		}
	}	
}

/***** function for command dump*****/
void command_dump(int start, int end) {
	static int last_address = 0;
	int tmp_idx;
	// case 1: dump
	if(start == -1 && end == -1) {
		if(last_address + 159 > 0xFFFFF) {
			tmp_idx = 0xFFFFF;
		}
		else tmp_idx = last_address + 159;
		print_memory(last_address, tmp_idx);
		last_address += 160;
	}
	// case 2: dump [start]
	else if(end == -1) {
		if(start + 159 > 0xFFFFF) {
			end = 0xFFFFF;
		}
		else {
			end = start + 159;
		}
		print_memory(start, end);
		last_address = end + 1;
	}
	// case 3: dump [start], [end]
	else {
		print_memory(start, end);
		last_address = end + 1;
	}
	if(last_address > 0xFFFFF) {
		last_address = 0;
	}
}

/***** function for command edit *****/
void command_edit(int address, int value) {
	memory[address] = value;
}

/***** function for command fill *****/
void command_fill(int start, int end, int value) {
	int i;

	for(i = start; i <= end; i++) {
		memory[i] = value;
	}
}

/***** function for command reset *****/
void command_reset() {
	free(memory);
	memory = (unsigned char*)calloc(MEMORY_SIZE, sizeof(unsigned char));
}

/***** function for opcode mnemonic *****/
void command_opcode() {
}

/***** function for opcodelist *****/
void command_opcodelist() {
}

void print_memory(int start, int end) {
	int i = 0, j = 0;

	int start_row = start/16,	end_row = end/16 + 1;
	int start_col = start % 16,	end_col = end % 16;
	int current_row, current_memory, tmp_memory;

	current_row = start - start_col;
	current_memory = start;
	for(i = start_row; i < end_row; i++) {
		printf("%05X ", current_row);

		tmp_memory = current_memory;
		for(j = 0; j < 16; j++) {
			if(start_row == end_row - 1) {
				if(j >= start_col && j <= end_col) {
					printf(" %02X", memory[current_memory]);
					current_memory++;
				}
				else {
					printf("   ");
				}
			}
			else if(i == start_row) {
				if(j < start_col) {
					printf("   ");
				}
				else {
					printf(" %02X", memory[current_memory]);
					current_memory++;
				}
			}
			else if(i == end_row - 1) {
				if(j > end_col) {
					printf("   ");
				}
				else {
					printf(" %02X", memory[current_memory]);
					current_memory++;
				}
			}
			else {
				printf(" %02X", memory[current_memory]);
				current_memory++;
			}
		}
		printf(" ; ");
		current_memory = tmp_memory;
		for(j = 0; j < 16; j++) {
			if(start_row == end_row - 1) {
				if(j >= start_col && j <= end_col) {
					character_print(current_memory);
					current_memory++;
				}
				else {
					printf(".");
				}
			}
			else if(i == start_row) {
				if(j < start_col) {
					printf(".");
				} 
				else {
					character_print(current_memory);
					current_memory++;
				}
			} 
			else if(i == end_row - 1) {
				if(j > end_col) {
					printf(".");
				} 
				else {
					character_print(current_memory);
					current_memory++;
				}
			} 
			else {
				character_print(current_memory);
				current_memory++;
			}
		}
		current_row += 16;
		printf("\n");
	}
}
void character_print(int idx) {
	if(0x20 <= memory[idx] && memory[idx] <= 0x7E) {
		printf("%c", memory[idx]);
	}
	else {
		printf(".");
	}
}

void read_opcode(FILE *fp) {
	OPCODE_NODE *node;
	int i, token_idx = 0, word_num = 3, idx = 0, format_num = 0, sum_char = 0, hash_idx, tmp = 0;
	char *end_of_file;
	char tmp_str[MAX_INPUT_LEN], **tokenize;
	bool word[100] = { false };

	while(1) {
		end_of_file = fgets(tmp_str, sizeof(tmp_str), fp);
		if(end_of_file == NULL) break;
		
		//create and initialize node
		node = (OPCODE_NODE*)malloc(sizeof(OPCODE_NODE));
		node->link = NULL;
		for(i = 0; i < 5; i++) {
			node->format[i] = false;
		} 

		//create tokens and tokenize the word
		tokenize = (char**)malloc(sizeof(char*) * 3);
		for(i = 0; i < 3; i++)
			tokenize[i] = (char*)malloc(sizeof(char) * 10);

		for(i = 0; i < strlen(tmp_str); i++) {
			if(('!' <= tmp_str[i] && tmp_str[i] <= '~')) {
				word[i] = true;
			}
		}
		for(i = 0; i < strlen(tmp_str); i++){
			if(word[i] == true) {
				tokenize[idx][token_idx] = tmp_str[i];
				token_idx++;
			}
			if(word[i] == true && word[i+1] == false) { 
				idx++;
				token_idx = 0;
			}
			if(idx + 1 == word_num) break;
		} //tokenize words

		//find hash key
		sum_char = 0;
		for(i = 0; i < strlen(tokenize[0]); i++) {
			sum_char += (int)tokenize[0][i];
		}

		hash_idx = sum_char % 20;

		//save formats, mnemonics and key values
		if(strlen(tokenize[2]) > 1) {
			format_num = (int)(tokenize[2][0] - '0');
			node->format[format_num] = true;
			format_num = (int)(tokenize[2][2] - '0');
			node->format[format_num] = true;
		}
		else if(strlen(tokenize[2]) == 1) {
			format_num = (int)(tokenize[2][0] - '0');
			node->format[format_num] = true;
		}

		strcpy(node->mnemonic, tokenize[1]);
		node->opcode = (int)strtol(tokenize[0], NULL, 16);

		//initialize variables for next loop
		for(i = 0; i < 3; i++)
			free(tokenize[i]);
		free(tokenize);
		for(i = 0; i < 100; i++)
			word[i] = false;

	}
}
