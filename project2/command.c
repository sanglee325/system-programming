#include "20171635.h"

extern bool exit_flag;
extern HISTORY_LIST *history;

/*----------------------------------------------------------*/
/* function	: command_help									*/
/* object	: execute function for command help				*/
/* return	: none											*/
/*----------------------------------------------------------*/
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

/*----------------------------------------------------------*/
/* function	: command_dir									*/
/* object	: execute function for command dir				*/
/* 			  that list files in current directory			*/
/* return	: none											*/
/*----------------------------------------------------------*/
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
			if((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXOTH) || (buf.st_mode & S_IXGRP)) {
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

/*----------------------------------------------------------*/
/* function	: command_quit									*/
/* object	: execute function for command quit				*/
/* 			  that quits the program						*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_quit() {
	HISTORY_NODE *target, *temp;

	exit_flag = true;
	if(history != NULL) {
		target = history->head;
		//temp = target->link; 

		while(1) {
			temp = target->link;
			free(target);
			if(temp == NULL) break;
			target = temp;
		}
	}
	free(history);
}

/*----------------------------------------------------------*/
/* function	: command_history								*/
/* object	: execute function for command history			*/
/* 			  that previous valid commands					*/
/* return	: none											*/
/*----------------------------------------------------------*/
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

/*----------------------------------------------------------*/
/* function	: command_dump									*/
/* object	: execute function for command dump				*/
/* 			  shows memory values							*/
/* return	: none											*/
/*----------------------------------------------------------*/
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

/*----------------------------------------------------------*/
/* function	: command_edit									*/
/* object	: execute function for command edit				*/
/* 			  that changes the value of certain address		*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_edit(int address, int value) {
	memory[address] = value;
}

/***** function for command fill *****/
/*----------------------------------------------------------*/
/* function	: command_fill									*/
/* object	: execute function for command fill				*/
/* 			  that changes given value from selected address*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_fill(int start, int end, int value) {
	int i;

	for(i = start; i <= end; i++) {
		memory[i] = value;
	}
}

/*----------------------------------------------------------*/
/* function	: command_reset									*/
/* object	: execute function for command reset			*/
/* 			  that sets all memory value to 0				*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_reset() {
	free(memory);
	memory = (unsigned char*)calloc(MEMORY_SIZE, sizeof(unsigned char));
}

/*----------------------------------------------------------*/
/* function	: command_opcode								*/
/* object	: execute function for command opcode			*/
/* 			  that shows opcode for mnemonic				*/
/* return	: if opcode is found return true, 				*/
/* 			  else return false								*/
/*----------------------------------------------------------*/
bool command_opcode(char *mnemonic) {
	int i, sum = 0, hash_idx = 0;
	OPCODE_NODE *tmp_node;

	for(i = 0; i < strlen(mnemonic); i++) {
		sum += (int)mnemonic[i];
	}
	hash_idx = sum % OPCODE_HASH_TABLE_SIZE;

	tmp_node = table[hash_idx];
	while(1) {
		if(!strcmp(mnemonic, tmp_node->mnemonic)) {
			printf("opcode is %X.\n", tmp_node->opcode);
			break;
		}
		else {
			tmp_node = tmp_node->link;
			if(tmp_node == NULL) {
				return false;
			}
		}
	}
	return true;
}

/*----------------------------------------------------------*/
/* function	: command_opcodelist							*/
/* object	: execute function for command opcodelist		*/
/* 			  that list every opcode						*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_opcodelist() {
	OPCODE_NODE *tmp_node;
	int i;

	for(i = 0; i < OPCODE_HASH_TABLE_SIZE; i++) {
		printf("%d : ", i);
		tmp_node = table[i];
		while(1) {
			if(tmp_node == NULL) {
				break;
			}
			else {
				printf("[%s, %X]", tmp_node->mnemonic, tmp_node->opcode);
				if(tmp_node->link != NULL) {
					printf(" -> ");
					tmp_node = tmp_node->link;
				}
				else break;
			}
		}
		printf("\n");
	}
}

/*----------------------------------------------------------*/
/* function	: print_memory									*/
/* object	: prints memory in command dump					*/
/* return	: none											*/
/*----------------------------------------------------------*/
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
/*----------------------------------------------------------*/
/* function	: character_print								*/
/* object	: prints character which is 					*/
/* 			  in between 0x20 and 0x7E						*/
/* return	: none											*/
/*----------------------------------------------------------*/
void character_print(int idx) {
	if(0x20 <= memory[idx] && memory[idx] <= 0x7E) {
		printf("%c", memory[idx]);
	}
	else {
		printf(".");
	}
}

