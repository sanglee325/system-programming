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
	NODE *target, *temp;

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
	NODE *target;

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
void command_edit() {
}

/***** function for command fill *****/
void command_fill() {
}

/***** function for command reset *****/
void command_reset() {
}

/***** function for opcode mnemonic *****/
void command_opcode_mnemonic() {
}

/***** function for opcodelist *****/
void command_opcodelist() {
}

void print_memory(int start, int end) {
	int i = 0, j = 0;

	int start_row = start/16,	end_row = end/16 + 1;
	int start_col = start % 16,	end_col = end % 16;
	int current_row, current_memory;

	current_row = start - start_col;
	current_memory = start;
	if(start_row == end_row) {
		printf("%05x ", current_row);
		for(j = 0; j < 16; j++) {
			if(j < start_col || j > end_col) {
				printf("   ");
			}
			else {
				printf(" %02X", memory[current_memory]);
				current_memory++;
			}
		}
		printf(" ; ");
		for(j = 0; j < 16; j++) {
			if(j < start_col || j > end_col) {
				printf(".");
			}
			else if(0x20 <= memory[current_memory] && memory[current_memory] <= 0x7E) {
				printf("%c", memory[current_memory]);
				current_memory++;
			}
		}
		printf("\n");
	}
	else {
		for(i = start_row; i < end_row; i++) {
			printf("%05x ", current_row);

			for(j = 0; j < 16; j++) {
				if(i == start_row) {
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
			for(j = 0; j < 16; j++) {
				if(i == start_row) {
					if(j < start_col) {
						printf(".");
					} 
					else if(0x20 <= memory[current_memory] && memory[current_memory] <= 0x7E) {
						printf("%c", memory[current_memory]);
						current_memory++;
					}
					else {
						printf(".");
					}
				} 
				else if(i == end_row - 1) {
					if(j > end_col) {
						printf(".");
					} 
					else if(0x20 <= memory[current_memory] && memory[current_memory] <= 0x7E) {
						printf("%c", memory[current_memory]);
						current_memory++;
					}
					else {
						printf(".");
					}
				} 
				else {
					if(0x20 <= memory[current_memory] && memory[current_memory] <= 0x7E) {
						printf("%c", memory[current_memory]);
					} 
					else {
						printf(".");
					}
					current_memory++;
				}
			}
			current_row += 16;
			printf("\n");
		}
	}
}	
