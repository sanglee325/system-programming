#include "20171635.h"

char origin[500];
bool command_assemble(char *filename) {
	FILE *file_asm;
	int i, idx, token_idx, delimiter = 0, word_num = 2;
	int program_len = 0;
	char tokenize[5][100], tmp_name[500];
	bool word[MAX_INPUT_LEN] = { false, };
	bool flag_pass1 = false, flag_pass2 = false;

	strcpy(tmp_name, filename);
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

	if(delimiter == 1) {
		strcpy(origin, tokenize[0]); // copy filename (only)

		file_asm = fopen(filename, "r");
		if(file_asm == NULL) {
			printf("ERROR: FILE DOES NOT EXIST\n");
			return false;
		}
		flag_pass1 = assemble_pass1(file_asm, &program_len);
		fclose(file_asm);
		if(!flag_pass1) {
			return false;
		}
		flag_pass2 = assemble_pass2(program_len);
	}
	else {
		printf("ERROR: INVALID FILE TYPE\n");
		return false;
	}
	return true;
}

bool assemble_pass1(FILE* file_asm, int *program_len) {
	FILE *file_inter;
	char input_asm[200];
	int error = 0, LOCCTR = 0, prev_LOCCTR = 0, format = 0;
	static int line_num = 1, start_address;
	bool flag_opcode = false, flag_directive = false, flag_operand_directive = false, flag_end = false;
	SYMBOL_SET info_input;
	//OPTABLE data;

	info_input.symbol = info_input.mnemonic = info_input.operand = NULL;
	file_inter = fopen("inter.asm", "w");

	while(1) {
		fgets(input_asm, 200, file_asm);

		prev_LOCCTR = LOCCTR;
		if(!isComment_check(input_asm)){
			continue;
		}
		else {
			tokenize_input(input_asm, &info_input, &error);
			if(error) {
				printf("ERROR: SYNTAX INVALID in line #%d\n", line_num);
				fclose(file_inter);
				return false;
			}
		}
		if(!strcmp(info_input.mnemonic, "START")) {
			if(LOCCTR != 0) {
				fclose(file_inter);
				printf("ERROR: NO START DIRECTIVE in line #%d\n", line_num);
				return false;
			}
			else {
				if(info_input.operand == NULL) {
					start_address = 0;
					LOCCTR = 0;
				}
				else {
					LOCCTR = (int)strtol(info_input.operand, NULL, 16);
					start_address = LOCCTR;
				}
			}
		}
		if(!strcmp(info_input.mnemonic, "END")) {
			flag_end = isEND_check(file_asm);
			if(!flag_end) {
				printf("ERROR: SYNTAX ERROR: END in line #%d\n", line_num);
				fclose(file_inter);
				return false;
			}
			if(info_input.symbol) {
				printf("ERROR: SYNTAX ERROR: END in line #%d\n", line_num);
				fclose(file_inter);
				return false;
			}
			fprintf(file_inter, "%d\t%X\t%d\t%s\t%s\t%s\n", line_num, prev_LOCCTR, format, info_input.symbol, info_input.mnemonic, info_input.operand);
			*program_len = LOCCTR - start_address;
			fclose(file_inter);
			return true;
		}
		if(info_input.symbol) {
			add_SYMBOL(&info_input, LOCCTR, &error);
			if(error) {
				printf("ERROR: SYMBOL OVERLAP in line #%d\n", line_num);
				fclose(file_inter);
				return false;
			}
		}
		flag_opcode = isOpcode_check(info_input.mnemonic, &format);
		if(!flag_opcode && format == 4) {
			printf("ERROR: INVALID FORMAT NUMBER in line #%d\n", line_num);
			fclose(file_inter);
			return false;
		}
		if(!flag_opcode) {
			flag_directive = isDirective_check(info_input.mnemonic);
			if(!flag_directive) {
				printf("ERROR: INVALID MNEMONIC in line #%d\n", line_num);
				fclose(file_inter);
				return false;
			}
			else if(flag_directive) {
				flag_operand_directive = operand_directive(&info_input, &LOCCTR, line_num);
				if(!flag_operand_directive) fclose(file_inter);
			}
		}
		else if(flag_opcode) {
			LOCCTR += format;
		}
		// LOCCTR dont forget
		fprintf(file_inter, "%d\t%X\t%d\t%s\t%s\t%s\n", line_num, prev_LOCCTR, format, info_input.symbol, info_input.mnemonic, info_input.operand); 

		info_input.symbol = info_input.mnemonic = info_input.operand = NULL;
		format = 0; error = 0;
		line_num += 1;
	}

	fclose(file_inter);
	return true;
}

bool assemble_pass2(int program_len) {
	FILE *inter, *object, *lst;
	char input[200] = { 0, }, *ptr;
	char line_num[10] = { 0, }, LOCCTR[10] = { 0 ,}, format[2] = { 0 ,};
	char label[10] = { 0 ,}, mnemonic[10] = { 0 ,}, operand[50] = { 0 ,};
	char file_obj[100], file_lst[100];
	int start_address = 0;
	SYMBOL_SET info_input;
	
	strcpy(file_obj, origin);
	strcat(file_obj, ".obj");
	strcpy(file_lst, origin);
	strcat(file_lst, ".lst");

	info_input.symbol = info_input.mnemonic = info_input.operand = NULL;
	inter = fopen("inter.asm", "r");
	object = fopen(file_obj, "w");
	lst = fopen(file_lst, "w");

	fgets(input, 200, inter);
	while(1) {
		ptr = strtok(input, "\t");
		strcpy(line_num, ptr);
		ptr = strtok(NULL, "\t");
		strcpy(LOCCTR, ptr);
		ptr = strtok(NULL, "\t");
		strcpy(format, ptr);
		ptr = strtok(NULL, "\t");
		strcpy(label, ptr);
		ptr = strtok(NULL, "\t");
		strcpy(mnemonic, ptr);
		ptr = strtok(NULL, "\t");
		strcpy(operand, ptr);

		if(!strcmp(mnemonic, "START")) {
			start_address =(int)strtol(LOCCTR, NULL, 16);
			
		}
		

		fgets(input, 200, inter);

	
		//printf("line %s: %s %s %s %s (format: %s)\n", line_num, LOCCTR, label, mnemonic, operand, format);
		

		if(!strcmp(mnemonic, "END"))	break;
	}
	fclose(object);
	fclose(lst);
	fclose(inter);

	return true;

}

void tokenize_input(char *input_asm, SYMBOL_SET *info, int *error) {
	int i, idx, token_idx, word_num = 0;
	int flag_label;
	char token[50][100] = { 0, };
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

	flag_label = isLabel_check(token[0], token[1]);
	if(flag_label == 1) {
		info->symbol = token[0];
		info->mnemonic = token[1];
		for(i = 3; i < 50; i++) {
			if(token[i][0] != 0) {
				strcat(token[2], " ");
				strcat(token[2], token[i]);
			}
			else break;
		}
		info->operand = token[2];
	}
	else if(flag_label == 0) {
		info->symbol = NULL;
		info->mnemonic = token[0];
		for(i = 2; i < 50; i++) {
			if(token[i][0] != 0) {
				strcat(token[1], " ");
				strcat(token[1], token[i]);
			}
			else break;
		}
		info->operand = token[1];
	}
	else if(flag_label == -1) {
		*error = 1;
	}
}

int isLabel_check(const char *token0, const char *token1) {
	int tmp;
	bool flag_opcode = false, flag_directive = false;
	
	flag_opcode = isOpcode_check(token0, &tmp);
	if(!flag_opcode) {
		flag_directive = isDirective_check(token0);
	}
	// check if first word is label or not
	
	if(flag_opcode || flag_directive) {
		return 0;
	}
	if(!flag_opcode && !flag_directive) {
		flag_opcode = false;
		flag_opcode = isOpcode_check(token1, &tmp);
		if(!flag_opcode) {
			flag_directive = isDirective_check(token1);
		}

		if(flag_opcode || flag_directive) return 1;
		else {
			return -1;
		}
	}	

	return true;
}

bool isOpcode_check(const char *token, int *format) {
	int i = 0, optable_idx = 0;
	char token_cp[10] = { 0 ,};
	OPCODE_NODE *op_tmp = NULL;
	
	if(token[0] == '+') { 
		*format = 4;
		for(i = 1; i < strlen(token); i++) {
			token_cp[i - 1] = token[i];
		}
	}
	else {
		*format = 0;
		strcpy(token_cp, token);
	}
	for(i = 0; i < strlen(token_cp); i++) {
		optable_idx += (int)token_cp[i];
	}
	optable_idx %= OPCODE_HASH_TABLE_SIZE;
	op_tmp = table[optable_idx];

	while(op_tmp) {
		if(!strcmp(token_cp, op_tmp->mnemonic)) {
			if(*format == 4) {
				if(!op_tmp->format[4]) return false;
				else if(op_tmp->format[4]) return true;
			}
			for(i = 0; i < 5; i++) {
				if(op_tmp->format[i]) { 
					*format = i;
					break;
				}
			}
			if(*format == 0) return false;
			return true;
		}
		else {
			op_tmp = op_tmp->link;
		}
	} 
	return false;
}

bool isDirective_check(const char *token) {
	if(!strcmp(token, "START")) {
		return true;
	}
	else if(!strcmp(token, "WORD")) {
		return true;
	}
	else if(!strcmp(token, "BYTE")) {
		return true;
	}
	else if(!strcmp(token, "RESW")) {
		return true;
	}
	else if(!strcmp(token, "RESB")) {
		return true;
	}
	else if(!strcmp(token, "END")) {
		return true;
	}
	else if(!strcmp(token, "BASE")) {
		flag_base = true;
		return true;
	}
	else if(!strcmp(token, "NOBASE")) {
		flag_base = false;
		return true;
	}
	return false;
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

bool isEND_check(FILE *fp) {
	int i;
	char input_asm[200];

	while(1){
		fgets(input_asm, 200, fp);

		if(feof(fp)) 
			return true;
		if(!isComment_check(input_asm))
			continue;
		else {
			for(i = 0; i < strlen(input_asm); i++) {
				if(input_asm[i] == ' ' || input_asm[i] == '\t' || input_asm[i] == '\n')
					continue;
				else return false;
			}
		}
	}

}
void add_SYMBOL(SYMBOL_SET *info_input, int LOCCTR, int *error) {
	int i, dict_order;
	SYMBOL_TABLE *symb_tmp = NULL, *symb_prev = NULL, *new_node = NULL;
	bool flag_valid = false;

	i = (int)(info_input->symbol[0] - 'A');
	new_node = (SYMBOL_TABLE*)calloc(1, sizeof(SYMBOL_TABLE));
	new_node->link = NULL;
	if(symb_table[i] == NULL) {
		strcpy(new_node->symbol, info_input->symbol);
		new_node->LOCCTR = LOCCTR;
		new_node->link = NULL;
		symb_table[i] = new_node;
		flag_valid = true;
	}
	else {
		// checking symbol table
		symb_tmp = symb_table[i];
		while(symb_tmp) {
			if(!strcmp(info_input->symbol, symb_tmp->symbol)) {
				*error = 1;
			} // symbol already exist
			else {
				symb_tmp = symb_tmp->link;
			}
		}
		strcpy(new_node->symbol, info_input->symbol);
		new_node->LOCCTR = LOCCTR;

		// adding to symbol table in dictionary order
		symb_tmp = symb_table[i];
		while(1) {
			dict_order = strcmp(new_node->symbol, symb_tmp->symbol);
			if(dict_order > 0) {
				symb_prev = symb_tmp;
				symb_tmp = symb_tmp->link;
				if(symb_tmp == NULL) {
					symb_prev->link = new_node;
					flag_valid = true;
					break;
				}
			}
			else if(dict_order < 0) {
				if(symb_prev == NULL) {
					symb_tmp->link = new_node;
					flag_valid = true;
				}
				else {
					symb_prev->link = new_node;
					symb_tmp = symb_tmp->link;
					if(symb_tmp)  
						new_node->link = symb_tmp;
					flag_valid = true;
				}
				break;
			}
		}
	}
	if(!flag_valid) free(new_node);
}

bool operand_directive(SYMBOL_SET *info_input, int *LOCCTR, int line_num) {
	int allocate, idx = 0, i = 0, count = 0;
	char *operand_error, tmp_operand[100] = { 0, };
	if(!strcmp(info_input->mnemonic, "RESW")) {
		allocate = (int)strtol(info_input->operand, &operand_error, 10);
		if(*operand_error) {
			printf("ERROR: OPERAND SYNTAX in line #%d\n", line_num);
			return false;
		}
		*LOCCTR = *LOCCTR + allocate * 3;
	}
	else if(!strcmp(info_input->mnemonic, "RESB")) {
		allocate = (int)strtol(info_input->operand, &operand_error, 10);
		if(*operand_error) {
			printf("ERROR: OPERAND SYNTAX in line #%d\n", line_num);
			return false;
		}
		*LOCCTR = *LOCCTR + allocate;
	}
	else if(!strcmp(info_input->mnemonic, "BYTE")) {
		for(i = 0; i < strlen(info_input->operand); i++) {
			if(info_input->operand[i] == '\'') count++;
		}
		if(count == 2 && info_input->operand[1] == '\'' && 
				info_input->operand[strlen(info_input->operand) - 1] == '\'') { 
			if(info_input->operand[0] == 'C') {
				*LOCCTR += strlen(info_input->operand) - 3;
			}
			else if(info_input->operand[0] == 'X') {
				idx = 0;
				for(i = 2; i < strlen(info_input->operand) - 1; i++) {
					tmp_operand[idx] = info_input->operand[i];
					idx++;
				}
				allocate = (int)strtol(tmp_operand, &operand_error, 16);
				if(*operand_error) {
					printf("ERROR: DIRECTIVE in line #%d\n", line_num);
					return false;
				}
				else {
					if(idx % 2 == 0) 
						*LOCCTR += idx/2;
					else 
						*LOCCTR += idx/2 + 1;
				}
			}
		}
		else {
			printf("ERROR: DIRECTIVE SYNTAX in line #%d.\n", line_num);
			return false;
		}
	}
	else if(!strcmp(info_input->mnemonic, "WORD")) {
		*LOCCTR += 3;
	}
	else if(!strcmp(info_input->mnemonic, "BASE")) {
		*LOCCTR += 0;
	}

	return true;
}
