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
		program_len = extern_symtab->length;
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
	int loc = 1;
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
	bool flag_run = true, flag_bp = true;

	switch(command) {
		case 1: // run command
			flag_run = run_prog(progaddr);
			break;
		case 2: // display breakpoints
			display_bp();
			flag_bp = true;
			break;
		case 3: // clear or add breakpoints
			flag_bp = set_bp(input[1]);
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

	printf("ERROR: COMMAND NUMBER INVALID\n");
	return false;
}

bool run_prog(int progaddr) {
	unsigned int objcode = 0;
	unsigned char opcode = 0;
	static int current_addr = 0, break_addr = 0x100000;
	static int reg[10] = { 0, };
	static bool on_bp = false;
	bool flag_opcode = false, flag_run = false, flag_val = false;
	int flag_n = 0, flag_i = 0, flag_x = 0, flag_b = 0, flag_p = 0;
	unsigned int disp;
	int opcode_format = 0;
	int i = 0, target_addr = 0, value = 0;
	int num_half_byte;
	// A: 0  X: 1  L: 2  B: 3  S: 4  T: 5  PC: 8  SW: 9

	typedef struct _opcode_set {
		int code;
		int format;
	}OPSET;

	reg[2] = 0xFFFFFF;
	OPSET opcode_set[NUM_OPCODE] = {
		{.code = 0x00, .format = 3}, {.code = 0x68, .format = 3}, {.code = 0x74, .format = 3},
		{.code = 0x50, .format = 3}, {.code = 0x0C, .format = 3}, // LDA LDB LDT LDCH STA
		{.code = 0x10, .format = 3}, {.code = 0x14, .format = 3}, {.code = 0x54, .format = 3}, 
		{.code = 0x3C, .format = 3}, {.code = 0x48, .format = 3}, // STX STL STCH J JSUB
		{.code = 0x38, .format = 3}, {.code = 0x30, .format = 3}, {.code = 0x4C, .format = 3}, 
		{.code = 0x28, .format = 3}, {.code = 0xA0, .format = 2}, // JLT JEQ RSUB COMP COMPR
		{.code = 0xB4, .format = 2}, {.code = 0xB8, .format = 2}, {.code = 0xE0, .format = 3}, 
		{.code = 0xD8, .format = 3}, {.code = 0xDC, .format = 3}  // CLEAR TIXR TD RD WD
	};

	if(!on_bp) {
		reg[2] = progaddr + program_len;
		reg[8] = current_addr = execaddr;
	}

	while(1) {
		flag_opcode = false; flag_val = false;
		objcode = 0; opcode_format = 0;
		disp = 0;

		if(reg[8] == 0xFFFFFF || reg[8] == progaddr + program_len) {
			print_prog_end(reg);
			for(i = 0; i < 10; i++) 
				reg[i] = 0;
			printf("End program\n");
			on_bp = false;
			return true;
		}

		current_addr = reg[8];

		printf("current: %X\n", current_addr);
		print_prog_end(reg);
		char c;
		scanf("%c", &c);

		if(flag_breakpoint) {
			if(breakpoints[current_addr]) {
				if(break_addr != current_addr) {
					break_addr = current_addr;
					print_prog_end(reg);
					on_bp = true;
					printf("Stop at checkpoint[%04X]\n", current_addr);
					return true;
				}
				else {
					break_addr = MEMORY_SIZE;
				}
			}
		}

		opcode = memory[current_addr] - memory[current_addr] % 4;

		for(i = 0; i < 20; i++) {
			if(opcode == opcode_set[i].code) {
				flag_opcode = true;
				opcode_format = opcode_set[i].format;
				break;
			}
		}

		if(!flag_opcode) {
			printf("ERROR: OPCODE INVALID\n");
			return false;
		}

		objcode = memory[current_addr] * 0x100 + memory[current_addr+1];

		if(opcode_format == 2) {
			num_half_byte = 4;
		}
		else if(opcode_format == 3) {
			objcode = objcode * 0x100 + memory[current_addr+2];
			num_half_byte = 6;
			if(objcode & 0x001000) {
				opcode_format = 4;
				objcode = objcode * 0x100 + memory[current_addr+3];
				num_half_byte = 8;
			}
		}	

		current_addr = reg[8] += opcode_format; //PC set

		if(opcode_format == 2) {
			flag_run = run_format2(opcode, objcode, reg);
		}
		else if(opcode_format == 3) {
			flag_n = objcode & 0x020000;
			flag_i = objcode & 0x010000;
			flag_x = objcode & 0x008000;
			flag_b = objcode & 0x004000;
			flag_p = objcode & 0x002000;
			disp = objcode & 0xFFF;

			if(disp & 0x800) {
				disp = disp | 0xFFFFF000;
			}
			if(flag_n && !(flag_i)) {
				target_addr = memory[disp]*0x10000 + memory[disp+1]*0x100 + memory[disp+2];
			} // indirect
			else if(!(flag_n) && flag_i) {
				value = disp;
				flag_val = true;
				if(flag_p) {
					value += reg[8];
				}
				else if(flag_b) {
					value += reg[3];
				}
			} // immediate

			if(flag_p) {
				target_addr = (int)disp + reg[8];
			}
			else if(flag_b) {
				target_addr = disp + reg[3];
			}
			
			if(flag_x) {
				target_addr += reg[1];
			}
			flag_run = run_format34(opcode, value, flag_val, opcode_format, target_addr, num_half_byte, &current_addr, reg);

		}
		else if(opcode_format == 4) {
			flag_n = objcode & 0x02000000;
			flag_i = objcode & 0x01000000;
			flag_x = objcode & 0x00800000;
			flag_b = objcode & 0x00400000;
			flag_p = objcode & 0x00200000;
			disp = objcode & 0xFFFFF;

			if(flag_n && !(flag_i)) {
				target_addr = memory[disp]*0x10000 + memory[disp+1]*0x100 + memory[disp+2];
			} // indirect
			else if(!(flag_n) && flag_i) {
				value = disp;
				flag_val = true;
				if(flag_p) {
					value += reg[8];
				}
				else if(flag_b) {
					value += reg[3];
				}
			} // immediate

			if(flag_p) {
				target_addr = (int)disp + reg[8];
			}
			else if(flag_b) {
				target_addr = disp + reg[3];
			}
			else if(!(flag_b) && !(flag_p)) {
				target_addr = disp + progaddr;
			}

			if(flag_x) {
				target_addr += reg[1];
			}
			flag_run = run_format34(opcode, value, flag_val, opcode_format, target_addr, num_half_byte, &current_addr, reg);
			
		}

		if(!flag_run) {
			return false;
		}
	}
	return true;
}

void print_prog_end(int *reg) {
	printf("A : %06X X : %06X\n", reg[0], reg[1]);
	printf("L : %06X PC: %06X\n", reg[2], reg[8]);
	printf("B : %06X S : %06X\n", reg[3], reg[4]);
	printf("T : %06X\n", reg[5]);
}

bool run_format2(int opcode, int objcode, int *reg) {
	int temp;
	int reg1, reg2;

	if(opcode == 0xB4) { // CLEAR
		temp = objcode & 0x00F0;
		temp = temp >> 4;
		if(temp < 0 || temp > 9) {
			printf("ERROR: object code CLEAR\n");
			return false;
		}
		reg[temp] = 0;
		return true;
	}
	else if(opcode == 0xA0) { //COMPR
		reg1 = objcode & 0x00F0;
		reg2 = objcode & 0x000F;

		reg1 = reg1 >> 4;
		if(reg1 < 0 || reg1 > 9 || reg2 < 0 || reg2 > 9) {
			printf("ERROR: REGISTER NUMBER IN COMPR\n");
			return false;
		}
		if(reg[reg1] < reg[reg2]) {
			reg[9] = -1;
		}
		else if(reg[reg1] == reg[reg2]) {
			reg[9] = 0;
		}
		else {
			reg[9] = 1;
		}
		return true;
	}
	else if(opcode == 0xB8) {
		reg[1]++;
		reg1 = objcode & 0x00F0;
		reg1 = reg1 >> 4;

		if(reg[1] < reg[reg1]) {
			reg[9] = -1;
		}
		else if(reg[1] == reg[reg1]) {
			reg[9] = 0;
		}
		else {
			reg[9] = 1;
		}
		return true;
	}
	else {
		printf("ERROR: INVALID OPCODE IN FORMAT2\n");
		return false;
	}
}

bool run_format34(int opcode, int value, bool flag_i, int format, int address, int num_half_byte, int *curr, int *reg) {
	int tmp = 0;
	if(opcode == 0x00) { // LDA
		if(flag_i)
			reg[0] = value;
		else
			reg[0] = fetch_value(address, format, reg);
	}
	else if(opcode == 0x68) { // LDB
		if(flag_i)
			reg[3] = value;
		else
		reg[3] = fetch_value(address, format, reg);
	}
	else if(opcode == 0x74) { // LDT
		if(flag_i)
			reg[5] = value;
		else
		reg[5] = fetch_value(address, format, reg);
	}
	else if(opcode == 0x50) { // LDCH
		if(flag_i) {
			reg[0] = reg[0] & 0xFFFF00;
			reg[0] += value & 0x0000FF;
		}
		else {
			reg[0] = reg[0] & 0xFFFF00;
			reg[0] += fetch_value(address, format, reg) & 0x0000FF;
		}
	}
	else if(opcode == 0x0C) { // STA
		load_memory(address, num_half_byte, reg[0]);
		print_memory(address, address+16);
	}
	else if(opcode == 0x10) { // STX
		load_memory(address, num_half_byte, reg[1]);
		print_memory(address, address+16);
	}
	else if(opcode == 0x14) { // STL
		load_memory(address, num_half_byte, reg[2]);
		print_memory(address, address+16);
	}
	else if(opcode == 0x54) { // STCH
		load_memory(address, num_half_byte, reg[0] & 0x0000FF);
		print_memory(address, address+16);
	}
	else if(opcode == 0x3C) { // J
		reg[8] = address;
	}
	else if(opcode == 0x48) { // JSUB
		reg[2] = reg[8];
		reg[8] = address;
	}
	else if(opcode == 0x38) { // JLT
		if(reg[9] < 0) {
			reg[8] = address;
		}
	}
	else if(opcode == 0x30) { // JEQ
		if(reg[9] == 0) {
			reg[8] = address;
		}
	}
	else if(opcode == 0x4C) { // RSUB
		*curr = reg[8] = reg[2];
	}
	else if(opcode == 0x28) { // COMP
		if(flag_i) {
			if(reg[0] < value) {
				reg[9] = -1;
			}
			else if(reg[0] == value) {
				reg[9] = 0;
			}
			else {
				reg[9] = 1;
			}
		}
		else {
			tmp = fetch_value(address, format, reg);
			if(reg[0] < tmp) {
				reg[9] = -1;
			}
			else if(reg[0] == tmp) {
				reg[9] = 0;
			}
			else {
				reg[9] = 1;
			}
		}
	}
	else if(opcode == 0xE0) { // TD
		reg[9] = -1;
	}
	else if(opcode == 0xD8) { // RD
		reg[0] = 0;
	}
	else if(opcode == 0xDC) { // WD
	}
	else {
		printf("%X %d\n", opcode, address);
		printf("ERROR: OPCODE INVALID IN FORMAT 3/4\n");
		return false;
	}
	return true;
}

int fetch_value(int addr, int format, int *reg) {
	int value = 0;
	
	value = memory[addr]*0x10000 + memory[addr+1]*0x100 + memory[addr+2];

	return value;
}

void display_bp() {
	int idx = 0;

	if(!flag_breakpoint) {
		printf("no breakpoints set.\n");
		return;
	}

	printf("breakpoint\n");
	printf("----------\n");

	for(idx = 0; idx < MEMORY_SIZE; idx++) {
		if(breakpoints[idx]) {
			printf("%04X\n", idx);
		}
	}
}

bool set_bp(char *input) {
	int i = 0, address = 0;
	char *error;
	
	if(!strcmp(input, "clear")) {
		if(flag_breakpoint) {
			for(i = 0; i < MEMORY_SIZE; i++) {
				breakpoints[i] = 0;
			}
			flag_breakpoint = false;
			printf("[ok] clear all breakpoints\n");
		}
		else {
			printf("no breakpoints set.\n");
		}

		return true;
	}

	address = (int)strtol(input, &error, 16);
	if(*error) {
		printf("ERROR: INVALID FORMAT OF ADDRESS\n");
		return false;
	}
	if(!(0 <= address && address <= 0xFFFFF)) {
		printf("ERROR: OVER BOUNDARY\n");
		return false;
	}
	else {
		flag_breakpoint = true;
		breakpoints[address] = 1;
		printf("[ok] create breakpoint %04X\n", address);
		return true;
	}
}

