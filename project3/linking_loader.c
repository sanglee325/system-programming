#include "20171635.h"

/*----------------------------------------------------------*/
/* function	: command_progaddr								*/
/* object	: sets the start address of where to load prog	*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_progaddr(int address) {
	progaddr = address;
	printf("Program starting address set to 0x%x.\n", address);
}

/*----------------------------------------------------------*/
/* function	: command_loader								*/
/* object	: loads program on the memory					*/
/* return	: true: load success/false: load fail			*/
/*----------------------------------------------------------*/
bool command_loader(int file_num, char input[][MAX_INPUT_LEN]) {
	char **filename;
	int i = 0;
	bool flag_pass1 = true, flag_pass2 = true;
	ESTAB *extern_symtab = NULL;

	filename = (char**)malloc(sizeof(char*) * file_num);
	for(i = 0; i < file_num; i++) {
		filename[i] = (char*)malloc(sizeof(char) * 50);
		strcpy(filename[i], input[i+1]);
	}

	extern_symtab = (ESTAB*)calloc(file_num, sizeof(ESTAB));

	flag_pass1 = linking_loader_pass1(progaddr, file_num, filename, extern_symtab);
	if(flag_pass1) {
		flag_pass2 = linking_loader_pass2(progaddr, file_num, filename, extern_symtab);
	}
	else {
		return false;
	}

	for(i = 0; i < file_num; i++) {
		free(filename[i]);
	}
	free(filename);

	if(flag_pass2) {
		print_control_section_table(file_num, extern_symtab);
		dealloc_extern_symbol_table(extern_symtab, file_num);
		return true;
	}
	else {
		return false;
	}	

}

bool linking_loader_pass1(int progaddr, int file_num, char **filename, ESTAB *extern_symbol_tab) {
	FILE *obj_file = NULL;
	char obj_line[150] = { 0, };
	char record_type = 0, prog_name[10] = { 0, };
	char extern_symbol[10] = { 0, };
	int start_addr = 0, CSEC_len = 0, file_seq = 0;
	int CSEC_addr = progaddr;
	int D_rec_loc = 0, D_rec_len = 0, tmp_addr = 0;
	int i = 0, buf;
	bool flag_search_csec = false, flag_search_estab = false;

	while(1) {
		if(file_seq == file_num) break;

		obj_file = fopen(filename[file_seq], "r");
		if(obj_file == NULL) {
			printf("ERROR: FILE DOES NOT EXIST\n");
			return false;
		}

		fgets(obj_line, 150, obj_file);
		sscanf(obj_line, "%c", &record_type);
		
		if(record_type != 'H') {
			printf("ERROR: HEADER RECORD IN OBJECT FILE\n");
			fclose(obj_file);
			return false;
		}
		sscanf(obj_line, "%c%s%06X%06X", &record_type, prog_name, &start_addr, &CSEC_len);

		if(start_addr != 0) {
			CSEC_addr -= start_addr;
		}

		/* 같은 이름의 section이 있으면 에러처리*/
		flag_search_csec = search_control_section(extern_symbol_tab, file_num, prog_name);
		if(flag_search_csec) {
			printf("ERROR: same name of section exist\n");
			fclose(obj_file);
			return false;
		}
		else {
			strcpy(extern_symbol_tab[file_seq].control_section_name, prog_name);
			extern_symbol_tab[file_seq].address = CSEC_addr;
			extern_symbol_tab[file_seq].length = CSEC_len;
		}

		while(record_type != 'E') {
			// initialize string
			for(i = 0; i < 150; i++) {
				obj_line[i] = 0;
			}

			fgets(obj_line, 150, obj_file);
			sscanf(obj_line, "%c", &record_type);

			if(record_type == 'D') {
				D_rec_loc = 1;
				D_rec_len = strlen(obj_line) - 1;

				while(D_rec_loc < D_rec_len) {
					tmp_addr = 0x1000000;
					sscanf(obj_line + D_rec_loc, "%6s%06X", extern_symbol, &tmp_addr);
					if(tmp_addr == 0x1000000) {
						printf("ERROR: DEFINE RECORD IN OBJECT FILE\n");
					}

					// same existence of symbol name
					flag_search_estab = search_estab_symbol(extern_symbol_tab, file_num, extern_symbol, &buf);
					if(flag_search_estab) {
						printf("ERROR: EXTERN SYMBOL TABLE\n");
						fclose(obj_file);
						return false;
					}
					else {
						add_symbol_extern_symtab(&extern_symbol_tab[file_seq].extern_symbol, extern_symbol, tmp_addr+CSEC_addr);
					}

					D_rec_loc += 12;
				}
			}
		}
		CSEC_addr += CSEC_len;

		if(start_addr != 0) {
			CSEC_addr += start_addr;
		}

		file_seq += 1;
		fclose(obj_file);
	}

	return true;
}
bool linking_loader_pass2(int progaddr, int file_num, char **filename, ESTAB *extern_symbol_tab) {
	FILE *obj_file;
	char record_type = 0, prog_name[10] = { 0, }, reference_tab[257] = { 0, };
	char reference_name[7];
	char operator = 0;
	char obj_line[150] = { 0, };
	int CSEC_addr = progaddr, CSEC_len = 0, tmp_addr = 0, mod_addr;
	int file_seq = 0, line_idx = 0, line_limit = 0;
	int objline_len = 0, str_len = 0, i = 0, refer_idx = 0, refer_addr = 0;
	int start_addr = 0, num_half_byte = 0, code_hex = 0;
	bool flag_search = false;

	execaddr = progaddr;

	while(1) {
		if(file_seq == file_num) break;

		obj_file = fopen(filename[file_seq], "r");

		for(i = 0; i < 257; i++) {
			reference_tab[i] = 0;
		}

		refer_idx = 2;
		reference_tab[1] = extern_symbol_tab[file_seq].address;

		fgets(obj_line, 150, obj_file);
		sscanf(obj_line, "%c", &record_type);

		sscanf(obj_line, "%c%s%06X%06X", &record_type, prog_name, &start_addr, &CSEC_len);

		while(record_type != 'E') {
			for(i = 0; i < 150; i++) {
				obj_line[i] = 0;
			}

			fgets(obj_line, 150, obj_file);
			sscanf(obj_line, "%c", &record_type);

			if(record_type == 'R') {
				str_len = strlen(obj_line);
				obj_line[str_len--] = 0;

				for(i = 1; i < str_len; i += 8) {
					sscanf(obj_line + i, "%02X%6s", &refer_idx, reference_name);
					flag_search = search_estab_symbol(extern_symbol_tab, file_num, reference_name,&refer_addr);

					if(flag_search) {
						reference_tab[refer_idx] = refer_addr;
					}
					else {
						printf("ERROR: REFERENCE RECORD INVALID\n");
						fclose(obj_file);
						return false;
					}
				}
			}
			if(record_type == 'T') {
				sscanf(obj_line, "T%06X%02X", &tmp_addr, &objline_len);
				tmp_addr += CSEC_addr;
				line_limit = 2 * objline_len + 9;
				
				for(line_idx = 9; line_idx < line_limit; line_idx += 2) {
					if(line_idx > 150) {
						printf("ERROR: OBJECT CODE LINE LIMIT\n");
						fclose(obj_file);
						return false;
					}
					if(tmp_addr > MEMORY_SIZE || tmp_addr < 0) {
						printf("ERROR: MEMORY OVERFLOW\n");
						fclose(obj_file);
						return false;
					}
					
					sscanf(obj_line + line_idx, "%02X", &code_hex);
					memory[tmp_addr] = code_hex;
					tmp_addr++;
				}
			}
			if(record_type == 'M') {
				sscanf(obj_line, "M%06X%02X%c%02X", &start_addr, &num_half_byte, &operator, &line_idx);
				start_addr += CSEC_addr;
				mod_addr = count_modif(start_addr, num_half_byte);

				if(operator == '+') {
					mod_addr += reference_tab[line_idx];
				}
				else if(operator == '-') {
					mod_addr -= reference_tab[line_idx];
				}

				load_memory(start_addr, num_half_byte, mod_addr);
			}
		}
		if(record_type == 'E') {
			sscanf(obj_line, "E%06X", &execaddr);
			execaddr += CSEC_addr;
		}

		CSEC_addr += CSEC_len;
		file_seq++;
		fclose(obj_file);
	}
	return true;
}
bool search_estab_symbol(ESTAB *extern_symbol_table, int file_num, char *str, int *addr) {
	EXT_SYMBOL *node = NULL;
	int i = 0;

	for(i = 0; i < file_num; i++) {
		node = extern_symbol_table[i].extern_symbol;
		while(node) {
			if(node->symbol && !strcmp(node->symbol, str)) {
				*addr = node->address;
				return true;
			}
			node = node->link;
		}
	}

	return false;
}
bool search_control_section(ESTAB *extern_symbol_table, int file_num, char *str) {
	int i = 0;
	for(i = 0; i < file_num; i++) {
		if(!extern_symbol_table[i].control_section_name) {
			return false;
		}
		if(!strcmp(extern_symbol_table[i].control_section_name, str)) {
			return true;
		}
	}

	return false;
}
void add_symbol_extern_symtab(EXT_SYMBOL **extern_symbol, char *str, int addr) {
	EXT_SYMBOL *node = NULL, *link = *extern_symbol;

	node = (EXT_SYMBOL*)calloc(1, sizeof(EXT_SYMBOL));
	strcpy(node->symbol, str);
	node->address = addr;

	if(link) {
		while(link->link) {
			link = link->link;
		}
		link->link = node;
	}
	else {
		*extern_symbol = node;
	}
}

int count_modif(int address, int num_half_byte) {
	int objcode = 0, i = 0;

	if(num_half_byte % 2 == 1) {
		objcode = memory[address] % 16;
		num_half_byte--;
		address++;
	}
	for(i = 0; i < num_half_byte; i += 2) {
		objcode *= 256;
		objcode += memory[address];
		address++;
	}

	return objcode;
}

void load_memory(int address, int num_half_byte, int objcode) {
	int i = 0;
	int loc = 16;
	unsigned int un_objcode = objcode;

	for(i = 0; i < num_half_byte; i++) {
		loc *= 16;
	}

	if(num_half_byte % 2 == 1) {
		memory[address] &= 0x10;
		memory[address] |= ((un_objcode % loc) / (loc/16));
		loc /= 16;
		num_half_byte--;
		address++;
	}

	for(i = 0; i < num_half_byte; i+=2) {
		memory[address] = ((un_objcode % loc)/(loc/256));
		address++;
		loc /= 256;
	}
}

// print control section map
void print_control_section_table(int num, ESTAB *est) {
	int i = 0, total_len = 0;
	EXT_SYMBOL *node = NULL;

	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("----------------------------------------------------------\n");

	for(i = 0; i < num; i++) {
		printf("%-6s\t\t\t\t%4X\t\t%04X\n", est[i].control_section_name, est[i].address, est[i].length);
		node = est[i].extern_symbol;

		while(node) {
			printf("\t\t%-6s\t\t%4X\n", node->symbol, node->address);
			node = node->link;
		}

		total_len += est[i].length;
	}

	printf("----------------------------------------------------------\n");
	printf("\t\t\t\ttotal length\t%04X\n", total_len);
}
void dealloc_extern_symbol_table(ESTAB *est, int num) {
	EXT_SYMBOL *target, *next;
	int i = 0;

	for(i = 0; i < num; i++) {
		target = est[i].extern_symbol;
		while(target) {
			next = target->link;
			free(target);
			target = next;
		}
	}
	free(est);
}

bool command_run_bp(int command, char input[][MAX_INPUT_LEN]) {
	static unsigned char* breakpoints = NULL;
	bool flag_run = true, flag_bp = true;

	if(!breakpoints) {
		breakpoints = (unsigned char*)calloc(MEMORY_SIZE, sizeof(unsigned char));
	}

	switch(command) {
		case 1: // run command
			break;
		case 2: // display breakpoints
			display_bp(breakpoints);
			flag_bp = true;
			break;
		case 3: // clear or add breakpoints
			flag_bp = set_bp(breakpoints, input[1]);
			break;
	}

	if(command == 1) {
		if(!flag_run) return false;
		else return true;
	}
	else if(command == 2) {
		return true;
	}
	else if(command == 3) {
		if(!flag_bp) return false;
		else return true;
	}

}

void display_bp(const unsigned char* breakpoints) {
	int idx = 0;

	printf("breakpoint\n");
	printf("----------\n");

	for(idx = 0; idx < MEMORY_SIZE; idx++) {
		if(breakpoints[idx]) {
			printf("%04X\n", idx);
		}
	}
}

bool set_bp(unsigned char* breakpoints, char *input) {
	int i = 0, address = 0;
	char *error;
	
	if(!strcmp(input, "clear")) {
		for(i = 0; i < MEMORY_SIZE; i++) {
			breakpoints[i] = 0;
		}
		printf("[ok] clear all breakpoints\n");
		return true;
	}
	
	address = (int)strtol(input, &error, 16);
	if(error) {
		printf("ERROR: INVALID FORMAT OF ADDRESS\n");
		return false;
	}
	if(!(0 <= address && address <= 0xFFFFF)) {
		printf("ERROR: OVER BOUNDARY\n");
		return false;
	}
	else {
		breakpoints[address] = 1;
		printf("[ok] create breakpoint %04X\n", address);
		return true;
	}
}
