#include "20171635.h"

char origin[500];
bool command_assemble(char *filename) {
	FILE *file_asm;
	int i, j, idx, token_idx, delimiter = 0, word_num = 2;
	int program_len = 0;
	char **tokenize, tmp_name[500];
	bool word[MAX_INPUT_LEN] = { false, };
	bool flag_pass1 = false;

	strcpy(tmp_name, filename);
	tokenize = (char**)malloc(sizeof(char*) * 3);
	for(i = 0; i < 3; i++)
		tokenize[i] = (char*)calloc(100, sizeof(char));

	for(i = 0; i < strlen(tmp_name); i++) {
		if(('!' <= tmp_name[i] && tmp_name[i] <= '~')) {
			if(tmp_name[i] == '.') {
				delimiter++;
				word[i] = false;
			}
			else
				word[i] = true;
		}
	}
	idx = 0; token_idx = 0;
	for(i = 0; i < strlen(tmp_name); i++){
		if(word[i] == true) {
			tokenize[idx][token_idx] = tmp_name[i];
			token_idx++;
		}
		if(word[i] == true && word[i+1] == false) { 
			idx++;
			token_idx = 0;
		}
		if(idx == word_num) break;
	} //tokenize words

	if(!strcmp(tokenize[1], "asm") && delimiter == 1) {
		strcpy(origin, tokenize[0]); // copy filename (only)

		file_asm = fopen(filename, "r");
		if(file_asm == NULL) {
			printf("The file does not exist\n");
			for(i = 0; i < word_num; i++)
				free(tokenize[i]);
			free(tokenize);
			return false;
		}
		flag_pass1 = assemble_pass1(file_asm, &program_len);
		fclose(file_asm);
		if(!flag_pass1) {
			for(i = 0; i < word_num; i++)
				free(tokenize[i]);
			free(tokenize);
			return false;
		}
	}
	else {
		printf("Invalid File type!\n");
		for(i = 0; i < word_num; i++)
			free(tokenize[i]);
		free(tokenize);
		return false;
	}
}

bool assemble_pass1(FILE* file_asm, int *program_len) {
	FILE *file_inter, *file_lst, *file_obj;
	char input_asm[200], *end_of_file;
	int i, error = 0, dict_order, optable_idx = 0, optable_cmp = -1;
	int start_address, LOCCTR = 0, line_num = 5, operand = 0;
	int prev_LOCCTR = 0;
	bool flag_op = false;
	SYMBOL_SET info_input;
	SYMBOL_TABLE *symb_tmp = NULL, *symb_prev = NULL, *new_node = NULL;
	OPCODE_NODE *op_tmp = NULL;
	OPTABLE data;

	info_input.symbol = info_input.mnemonic = info_input.operand = NULL;
	file_inter = fopen("inter.asm", "w");

	while(1) {
		fgets(input_asm, 200, file_asm);
		
		if(!isComment_check(input_asm)){
			break;
		}
		else {
			tokenize_input(input_asm, &info_input, &error);
			if(error) {
				switch (error) {
					case 1: printf("Invalid Syntax\n");
							break;
					case 2: printf("Invalid Syntax: lowercase error\n");
							break;
				}
				fclose(file_inter);
				return false;
				// case of errors
			}
		}
		if(info_input.symbol) {
			i = (int)(info_input.symbol[0] - 'A');
			symb_tmp = symb_table[i];
			if(symb_tmp == NULL) {
				new_node = (SYMBOL_TABLE*)malloc(sizeof(SYMBOL_TABLE));
				strcpy(new_node->symbol, info_input.symbol);
				symb_table[i] = new_node;
			}
			else {
				// checking symbol table
				while(symb_tmp) {
					if(!strcmp(info_input.symbol, symb_tmp->symbol)) {
						fclose(file_inter);
						return false;
					}
					else {
						//prev_node = tmp_node;
						symb_tmp = symb_tmp->link;
					}
				}
				new_node = (SYMBOL_TABLE*)malloc(sizeof(SYMBOL_TABLE));
				strcpy(new_node->symbol, info_input.symbol);
				
				// adding to symbol table in dictionary order
				symb_tmp = symb_table[i];
				while(1) {
					dict_order = strcmp(new_node->symbol, symb_tmp->symbol);
					if(dict_order == 1) {
						symb_prev = symb_tmp;
						symb_tmp = symb_tmp->link;
						if(symb_tmp == NULL) {
							symb_prev->link = new_node;
							break;
						}
					}
					else if(dict_order == -1) {
						symb_prev->link = new_node;
						new_node->link = symb_tmp;
						break;
					}
				}

			}
		}// LOCCTR dont forget
		if(!strcmp(info_input.mnemonic, "START")) {
			if(LOCCTR != 0) {
				fclose(file_inter);
				return false;
			}
			else {
				// save operand as starting address
				// init LOCCTR
				// read nextline
			}
		}
		if(info_input.mnemonic[0] == '+') {
			format_num = 4;
			i = 1;
		}
		else i = 0;
		for(; i < strlen(info_input.mnemonic); i++) {
			optable_idx += info_input.mnemonic[i];
		}
		optable_idx %= OPCODE_HASH_TABLE_SIZE;
		op_tmp = table[optable_idx];
		while(op_tmp) {
			if(!strcmp(info_input.mnemonic, op_tmp->mnemonic)) {
				flag_op = true;
				data.opcode = op_tmp->opcode;
				//LOCCTR += 3;
			}
			else {
				op_tmp = op_tmp->link;
			}
		}
		if(op_tmp == NULL) {
			if(!strcmp(info_input.mnemonic, "WORD")) {
				flag_op = true;
				LOCCTR += 3;
			}
			else if(!strcmp(info_input.mnemonic, "BYTE")) {
				flag_op = true;
			}
			else if(!strcmp(info_input.mnemonic, "RESW")) {
				flag_op = true;
			}
			else if(!strcmp(info_input.mnemonic, "RESB")) {
				flag_op = true;
			}
			else if(!strcmp(info_input.mnemonic, "END")) {
				flag_op = true;
				// end of program..?
			}
		}

		


		//prev_LOCCTR = LOCCTR = (int)strtol(, &error, 16);
		info_input.symbol = info_input.mnemonic = info_input.operand = NULL;

	}

	fclose(file_inter);

}

void tokenize_input(char *input_asm, SYMBOL_SET *info, int *error) {
	int i, idx, token_idx, word_num = 0;
	bool flag_label = false;
	char token[50][50];
	bool word[200] = { false, };

	for(i = 0; i < strlen(input_asm); i++) {
		if(input_asm[i] == '\t' || input_asm[i] == ' ') 
			word[i] = false;
		else 
			word[i] = true;
	}
	word[strlen(input_asm) - 1] = false;

	for(i = 1; i < strlen(input_asm); i++) {
		if(word[i-1] == true && word[i] == false) 
			word_num++;
	}

	idx = 0; token_idx = 0;
	for(i = 0; i < strlen(input_asm); i++){
		if(word[i] == true) {
			token[idx][token_idx] = input_asm[i];
			token_idx++;
		}
		if(word[i] == true && word[i+1] == false) { 
			idx++;
			token_idx = 0;
		}
		if(idx == word_num) break;
	} //tokenize words

	flag_label = isLabel_check(token, word_num);
	/*
	for(i = 0; i < strlen(input_asm); i++) {
		printf("%2c ", input_asm[i]);
	}
	for(i = 0; i < strlen(input_asm); i++) {
		printf("%2d ", (int)input_asm[i]);
	}
	printf("\n");
	*/
	

}

bool isLabel_check(const char **token, const int num) {
	int i, j, optable_idx = 0;
	bool no_label, label;
	OPCODE_NODE *op_tmp = NULL;

	for(i = 0; i < strlen(token[0]); i++) {
		optable_idx += token[0][i];
	}
	op_tmp = table[optable_idx];

	while(op_tmp) {
		if(!strcmp(token[0], op_tmp->mnemonic)) {
			no_label = true;
		}
		else {
			op_tmp = op_tmp->link;
		}
	}

	if(no_label) {
		optable_idx = 0;
		for(i = 0; i < strlen(token[1]); i++) {
			optable_idx += token[1][i];
		}
		op_tmp = table[optable_idx];

	}	

}
bool isComment_check(const char* input) {
	int i = 0;
	while(input[i] == ' ' || input[i] == '\t' || input[i] == '\n') i++;

	if(input[i] == '.') {
		return false;
	}
	else {
		return true;
	}
}
