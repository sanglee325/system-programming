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
bool command_loader(int file_num, char** input) {
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

	if(flag_pass2) {
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
	int i = 0;
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
				D_rec_len = strlen(obj_line);

				while(D_rec_loc < D_rec_len) {
					tmp_addr = 0x1000000;
					sscanf(obj_line + D_rec_loc, "%6s%06X", extern_symbol, &tmp_addr);
					if(tmp_addr == 0x1000000) {
						printf("ERROR: DEFINE RECORD IN OBJECT FILE\n");
					}

					// same existence of symbol name
					flag_search_estab = search_estab_symbol(extern_symbol_tab, file_num, extern_symbol);
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
	FILE *file_obj;
	char record_type = 0, prog_name[10] = { 0, };
	char tmp, tmp_oper = 0;
	char obj_line[150] = { 0, };
	int CSEC_addr = progaddr, CSEC_len = 0, tmp_addr = 0;
	int file_seq = 0;
	int objline_len = 0, str_len = 0;
	int start_addr = 0, num_half_byte = 0;
	bool flag_search;


	return true;
}
bool serach_estab_symbol(ESTAB *extern_symbol_table, int file_num, char *str) {
	EXT_SYMBOL *node = NULL;
	int i = 0;

	for(i = 0; i < file_num; i++) {
		node = extern_symbol_table[i].extern_symbol;
		while(node) {
			if(node->symbol && !strcmp(node->symbol, str)) {
				return true;
			}
			node = node->link;
		}
	}

	return false;
}
bool search_control_section(ESTAB *extern_symbol_table, int file_num, char *str) {
	int i = 0;
	for(i < 0; i < file_num; i++) {
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
