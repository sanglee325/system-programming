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
	int error = 0, LOCCTR = 0, prev_LOCCTR = 0, format = 0, opcode;
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
				printf("ERROR: SYNTAX INVALID: %s", input_asm);
				fclose(file_inter);
				return false;
			}
		}
		if(!strcmp(info_input.mnemonic, "START")) {
			if(LOCCTR != 0) {
				fclose(file_inter);
				printf("ERROR: NO START DIRECTIVE: %s", input_asm);
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
				printf("ERROR: SYNTAX ERROR: END: %s", input_asm);
				fclose(file_inter);
				return false;
			}
			if(info_input.symbol) {
				printf("ERROR: SYNTAX ERROR: END: %s", input_asm);
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
				printf("ERROR: SYMBOL OVERLAP: %s", input_asm);
				fclose(file_inter);
				return false;
			}
		}
		flag_opcode = isOpcode_check(info_input.mnemonic, &format, &opcode);
		if(!flag_opcode && format == 4) {
			printf("ERROR: INVALID FORMAT NUMBER: %s", input_asm);
			fclose(file_inter);
			return false;
		}
		if(!flag_opcode) {
			flag_directive = isDirective_check(info_input.mnemonic);
			if(!flag_directive) {
				printf("ERROR: INVALID MNEMONIC: %s", input_asm);
				fclose(file_inter);
				return false;
			}
			else if(flag_directive) {
				flag_operand_directive = operand_directive(&info_input, &LOCCTR, line_num);
				if(!flag_operand_directive) {
					printf("ERROR: OPERAND SYNTAX: %s", input_asm);
					fclose(file_inter);
					return false;
				}
			}
		}
		else if(flag_opcode) {
			flag_opcode = isFormat_check(format, info_input.mnemonic, info_input.operand);
			if(flag_opcode)
				LOCCTR += format;
			else {
				printf("ERROR: FORMAT SYNTAX: %s", input_asm);
				fclose(file_inter);
				return false;
			}
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
	char input[200] = { 0, };
	char cur_line_num[10] = { 0, }, cur_LOCCTR[10] = { 0 ,}, cur_format[2] = { 0 ,};
	char cur_label[10] = { 0 ,}, cur_mnemonic[10] = { 0 ,}, cur_operand[50] = { 0 ,};
	char next_line_num[10] = { 0, }, next_LOCCTR[10] = { 0 ,}, next_format[2] = { 0 ,};
	char next_label[10] = { 0 ,}, next_mnemonic[10] = { 0 ,}, next_operand[50] = { 0 ,};
	char file_obj[100], file_lst[100];
	int directive_code[30] = { 0, };
	int i, start_address = 0, tot_digits = 0, cur_digits;
	int opcode[8] = { 0, } , opcode_num = 0, format = 0, disp_add[20] = { 0, };
	bool flag_opcode = false, flag_directive = false, flag_bit_set = false;
	SYMBOL_SET info_input;
	REGISTER reg;
	FLAG_BIT nixbpe;
	
	strcpy(file_obj, origin);
	strcat(file_obj, ".obj");
	strcpy(file_lst, origin);
	strcat(file_lst, ".lst");

	info_input.symbol = info_input.mnemonic = info_input.operand = NULL;
	inter = fopen("inter.asm", "r");
	object = fopen(file_obj, "w");
	lst = fopen(file_lst, "w");

	fgets(input, 200, inter);
	tokenize_inter(input, cur_line_num, cur_LOCCTR, cur_format, cur_label, cur_mnemonic, cur_operand); 
	while(1) {
		flag_opcode = false; flag_directive = false; opcode_num = 0; format = 0;
		if(!strcmp(cur_mnemonic, "START")) {
			start_address =(int)strtol(cur_LOCCTR, NULL, 16);
			tot_digits = count_digits(program_len);
			cur_digits = count_digits(start_address);

			printf("%6s\t", cur_line_num);
			for(i = 0; i < tot_digits-cur_digits; i++) {
				printf("0");
			}
			printf("%s\t%6s\t%6s\t%6s", cur_LOCCTR, cur_label, cur_mnemonic, cur_operand);

			printf("H%-6s/", cur_label);
			for(i = 0; i < 6-cur_digits; i++) {
				printf("0");
			}
			printf("%X/", start_address);
			for(i = 0; i < 6-tot_digits; i++) {
				printf("0");
			}
			printf("%X\n", program_len);
			fgets(input, 200, inter);
			tokenize_inter(input, next_line_num, next_LOCCTR, next_format, next_label, next_mnemonic, next_operand);
			strcpy(cur_line_num, next_line_num);
			strcpy(cur_LOCCTR, next_LOCCTR);
			strcpy(cur_format, next_format);
			strcpy(cur_label, next_label);
			strcpy(cur_mnemonic, next_mnemonic);
			strcpy(cur_operand, next_operand);
			continue;
		}
		flag_opcode = isOpcode_check(cur_mnemonic, &format, &opcode_num);
		if(!flag_opcode) {
			flag_directive = isDirective_check(cur_mnemonic);
			if(flag_directive) {
				// make function for directive obj code
				directive_objcode(cur_mnemonic, cur_operand, directive_code);
			}
			printf("[format:%s]", cur_format);
			printf("Directive");
			for(i = 0; i < 30; i++) {
				printf("%X", directive_code[i]);
			}
			printf("\n");
		}
		else {
			num_to_binary(opcode, opcode_num, 8);
		}
		fgets(input, 200, inter);
		tokenize_inter(input, next_line_num, next_LOCCTR, next_format, next_label, next_mnemonic, next_operand);
		if(!flag_directive) {
			printf("[format:%s]", cur_format);
			reg.PC = (int)strtol(next_LOCCTR, NULL, 16);

			if(!strcmp(cur_mnemonic, "BASE")) {
				flag_base = true;
			}
			if(!strcmp(cur_mnemonic, "NOBASE")) {
				flag_base = false;
			}
			flag_bit_set = set_flagbit(&nixbpe, cur_label, cur_mnemonic, cur_format, cur_operand, reg.PC, disp_add);
			printf("opcode: %d%d%d%d%d%d%d%d | flag: %d%d%d%d%d%d | disp: %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d\n", opcode[0], opcode[1], opcode[2], opcode[3], opcode[4], opcode[5], opcode[6], opcode[7], nixbpe.n, nixbpe.i, nixbpe.x, nixbpe.b, nixbpe.p, nixbpe.e, disp_add[0], disp_add[1], disp_add[2], disp_add[3], disp_add[4], disp_add[5], disp_add[6], disp_add[7], disp_add[8], disp_add[9], disp_add[10], disp_add[11], disp_add[12], disp_add[13], disp_add[14], disp_add[15], disp_add[16], disp_add[17], disp_add[18], disp_add[19]);
			if(!flag_bit_set) {
				return false;
				// case for fail in making obj code must be added (ex. removing files, deleting symbols)
			}
		}
		strcpy(cur_line_num, next_line_num);
		strcpy(cur_LOCCTR, next_LOCCTR);
		strcpy(cur_format, next_format);
		strcpy(cur_label, next_label);
		strcpy(cur_mnemonic, next_mnemonic);
		strcpy(cur_operand, next_operand);

		//printf("line %s: %s %s %s %s (format: %s)\n", line_num, LOCCTR, label, mnemonic, operand, format);
		for(i = 0; i < 30; i++) {
			directive_code[i] = 0;
		}
		for(i = 0; i < 20; i++) {
			disp_add[i] = 0;
		}
		for(i = 0; i < 8; i++) {
			opcode[i] = 0;
		}
		nixbpe.n = nixbpe.i = nixbpe.x = nixbpe.b = nixbpe.p = nixbpe.e = 0;

		if(!strcmp(cur_mnemonic, "END"))	break;
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
	int tmp, opcode;
	bool flag_opcode = false, flag_directive = false;
	
	flag_opcode = isOpcode_check(token0, &tmp, &opcode);
	if(!flag_opcode) {
		flag_directive = isDirective_check(token0);
	}
	// check if first word is label or not
	
	if(flag_opcode || flag_directive) {
		return 0;
	}
	if(!flag_opcode && !flag_directive) {
		flag_opcode = false;
		flag_opcode = isOpcode_check(token1, &tmp, &opcode);
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

bool isOpcode_check(const char *token, int *format, int *opcode) {
	int i = 0, optable_idx = 0;
	char token_cp[10] = { 0 , };
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
			*opcode = op_tmp->opcode;
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

bool isFormat_check(int format, const char *mnemonic, const char *operand) {
	char copy_operand[50] = { 0, }, *ptr = NULL, *error = NULL;
	char reg1[20] = { 0, }, reg2[20] = { 0, };
	int n_bit = 0, i;

	strcpy(copy_operand, operand);
	if(format == 1) {
		if(operand) return false;	
	}
	else if(format == 2) {
		if(!strcmp(mnemonic, "ADDR") || !strcmp(mnemonic, "COMPR") || !strcmp(mnemonic, "DIVR") ||
				!strcmp(mnemonic, "MULR") || !strcmp(mnemonic, "RMO") || !strcmp(mnemonic, "SUBR")) {
			//operand must have 2 reg
			ptr = strtok(copy_operand, ",");
			strcpy(reg1, ptr);
			ptr = strtok(NULL, ",");
			strcpy(reg2, ptr);
			ptr = strtok(NULL, ",");
			if(ptr) return false; // more register
			if(reg2[0] == ' ') {
				for(i = 0; i < 20; i++){
					reg2[i] = reg2[i+1];
				}
			}

			if(!strcmp(reg1, "B") || !strcmp(reg1, "S") || !strcmp(reg1, "T") || !strcmp(reg1, "F")
					|| !strcmp(reg1, "A") || !strcmp(reg1, "X") || !strcmp(reg1, "L") 
					|| !strcmp(reg1, "PC") || !strcmp(reg1, "SW")) {
				if(!strcmp(reg2, "B") || !strcmp(reg2, "S") || !strcmp(reg2, "T") || !strcmp(reg2, "F")
						|| !strcmp(reg2, "A") || !strcmp(reg2, "X") || !strcmp(reg2, "L") 
						|| !strcmp(reg2, "PC") || !strcmp(reg2, "SW")) return true;
				else return false;
			}
			else return false;
		}
		if(!strcmp(mnemonic, "CLEAR") || !strcmp(mnemonic, "TIXR")) {
			ptr = strtok(copy_operand, ",");
			strcpy(reg1, ptr);
			ptr = strtok(NULL, ",");
			if(ptr) return false;

			if(!strcmp(reg1, "B") || !strcmp(reg1, "S") || !strcmp(reg1, "T") || !strcmp(reg1, "F")
					|| !strcmp(reg1, "A") || !strcmp(reg1, "X") || !strcmp(reg1, "L") 
					|| !strcmp(reg1, "PC") || !strcmp(reg1, "SW")) return true;
			else return false;
			// operand must have 1 reg
		}
		if(!strcmp(mnemonic, "SHIFTL") || !strcmp(mnemonic, "SHIFTR")) {
			ptr = strtok(copy_operand, ",");
			strcpy(reg1, ptr);
			ptr = strtok(NULL, ",");
			strcpy(reg2, ptr);
			ptr = strtok(NULL, ",");
			if(ptr) return false;
			if(reg2[0] == ' ') {
				for(i = 0; i < 20; i++){
					reg2[i] = reg2[i+1];
				}
			}

			if(!strcmp(reg1, "B") || !strcmp(reg1, "S") || !strcmp(reg1, "T") || !strcmp(reg1, "F")
					|| !strcmp(reg1, "A") || !strcmp(reg1, "X") || !strcmp(reg1, "L") 
					|| !strcmp(reg1, "PC") || !strcmp(reg1, "SW")) {
				n_bit = (int)strtol(reg2, &error, 10);
				if(*error) return false;
				else return true;
			}
			else return false;
			// r1 , r2 = n-1
		}
	}
	else if(format == 3 || format == 4) {
		if(!strcmp(mnemonic, "RSUB")) {
			if(copy_operand[0] == 0) return true;
			else return false;
		}
		ptr = strtok(copy_operand, ",");
		strcpy(reg1, ptr);
		if(!strcmp(reg1, "B") || !strcmp(reg1, "S") || !strcmp(reg1, "T") || !strcmp(reg1, "F")
				|| !strcmp(reg1, "A") || !strcmp(reg1, "X") || !strcmp(reg1, "L") 
				|| !strcmp(reg1, "PC") || !strcmp(reg1, "SW")) {
			return false;
		}
		if(!strcmp(reg1, operand)) {
			return true;
		}
		else {
			ptr = strtok(NULL, ",");
			strcpy(reg2, ptr);
			ptr = strtok(NULL, ",");
			if(ptr) return false;
			if(reg2[0] == ' ') {
				for(i = 0; i < 20; i++){
					reg2[i] = reg2[i+1];
				}
			}
			if(!strcmp(reg2, "X")) return true;
			else false;
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
			return false;
		}
		*LOCCTR = *LOCCTR + allocate * 3;
	}
	else if(!strcmp(info_input->mnemonic, "RESB")) {
		allocate = (int)strtol(info_input->operand, &operand_error, 10);
		if(*operand_error) {
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
					return false;
				}
				else {
					if(idx % 2 == 0) 
						*LOCCTR += idx/2;
					else 
						return false;
				}
			}
		}
		else {
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
void tokenize_inter(char *input, char *line_num, char *LOCCTR, char* format, char* label, char* mnemonic, char* operand) {
	char *ptr;
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
}
void copy_next_to_cur(char *line_num, char *LOCCTR, char* format, char* label, char* mnemonic, char* operand, char *line_num2, char *LOCCTR2, char* format2, char* label2, char* mnemonic2, char* operand2) {
	strcpy(line_num, line_num2);
	strcpy(LOCCTR, LOCCTR2);
	strcpy(format, format2);
	strcpy(label, label2);
	strcpy(mnemonic, mnemonic2);
	strcpy(operand, operand2);
}

int count_digits(int program_len) {
	int copy = program_len, digit = 0;

	while(1) {
		copy = copy/16;
		if(copy == 0) break;
		digit++;
	}
	return digit + 1;
}

void num_to_binary(int *opcode, int opcode_num, int size) {
	int position = size - 1, i;
	
	if(0 <= opcode_num) {
		while(1) {
			opcode[position] = opcode_num % 2;
			opcode_num = opcode_num / 2;

			position--;

			if(opcode_num == 0) break;
		}
	}
	else if(opcode_num < 0) {
		opcode_num *= -1;
		while(1) {
			opcode[position] = opcode_num % 2;
			opcode_num = opcode_num / 2;

			position--;

			if(opcode_num == 0) break;
		}
		opcode[0] = 1;
		for(i = size - 1; i > 0; i--) {
			if(opcode[i] == 0) opcode[i] = 1;
			else opcode[i] = 0;
		}
		for(i = size - 1; i > 0; i--) {
			if(opcode[i] + 1 == 2) {
				opcode[i] = 0;
			}
			else if(opcode[i] + 1 == 1) {
				opcode[i] += 1;
				break;
			}
		}
	
	}
}

bool set_flagbit(FLAG_BIT *nixbpe, char* symbol, char *mnemonic, char *format, char *operand, int PC, int *disp_add) {
	char copy_operand[50] = { 0, }, *ptr = NULL, *error = NULL, idx[20] = { 0, };
	char tmp_immediate[50] = { 0, };
	int i, immediate, symbol_address, indirect, simple, tmp;
	strcpy(copy_operand, operand);

	if(format[0] == '1' || format[0] == '2') return true;

	if(format[0] == '4') {
		nixbpe->b = 0;
		nixbpe->p = 0;
		nixbpe->e = 1;
	}
	else nixbpe->e = 0;

	if(!strcmp(mnemonic, "RSUB")) return true;

	if(operand[0] == '#') {
		nixbpe->n = 0;
		nixbpe->i = 1;
		for(i = 0; i < 50; i++) {
			tmp_immediate[i] = copy_operand[i+1];
			copy_operand[i] = copy_operand[i+1];
		}
		immediate = (int)strtol(tmp_immediate, NULL, 16);
		if(format[0] == '3') {
			if(-2048 <= immediate && immediate <= 2047) {
				num_to_binary(disp_add, immediate, 12);
				nixbpe->b = 0;
				nixbpe->p = 1;
			}
			else {
				if(flag_base) {
					if(0 <= immediate && immediate <= 4095) { 
						num_to_binary(disp_add, immediate, 12);
						nixbpe->b = 1;
						nixbpe->p = 0;
					}
					else return false;
				}
				else {
					return false;
				}
			}
		}
		else if(format[0] == '4') {
			num_to_binary(disp_add, immediate, 20);
		}
	}
	else if(operand[0] == '@') {
		nixbpe->n = 1;
		nixbpe->i = 0;
		// find symbol address and symb-pc = disp
		for(i = 0; i < 50; i++) {
			copy_operand[i] = copy_operand[i+1];
		}
		indirect = search_symbol(operand);
		tmp = indirect - PC;
		if(format[0] == '3') {
			if(-2048 <= tmp && tmp <= 2047) {
				num_to_binary(disp_add, tmp, 12);
				nixbpe->b = 0;
				nixbpe->p = 1;
			}
			else {
				if(flag_base) {
					if(0 <= indirect && indirect <= 4095) { 
						num_to_binary(disp_add, indirect, 12);
						nixbpe->b = 1;
						nixbpe->p = 0;
					}
					else return false;
				}
				else {
					return false;
				}
			}
		}
		else if(format[0] == '4') {
			num_to_binary(disp_add, indirect, 20);
		}
	}
	else {
		nixbpe->n = 1;
		nixbpe->i = 1;
		simple = search_symbol(operand);
		tmp = simple - PC;
		if(format[0] == '3') {
			if(-2048 <= tmp && tmp <= 2047) {
				num_to_binary(disp_add, tmp, 12);
				nixbpe->b = 0;
				nixbpe->p = 1;
			}
			else {
				if(flag_base) {
					if(0 <= simple && simple <= 4095) { 
						num_to_binary(disp_add, simple, 12);
						nixbpe->b = 1;
						nixbpe->p = 0;
					}
					else return false;
				}
				else {
					return false;
				}
			}
		}
		else if(format[0] == '4') {
			num_to_binary(disp_add, simple, 20);
		}

		//simple addressing
	}
	if(format[0] == '3' || format[0] == '4') {
		ptr = strtok(copy_operand, ",");
		ptr = strtok(NULL, ",");
		if(!ptr) {
			nixbpe->x = 0;
		}
		else {
			strcpy(idx, ptr);
			if(idx[0] == ' ') {
				for(i = 0; i < 20; i++){
					idx[i] = idx[i+1];
				}
			}
			if(!strcmp(idx, "X")) 
				nixbpe->x = 1;
			else 
				nixbpe->x = 0;
		}
	}
	return true;
}

int search_symbol(const char *symbol) {
	SYMBOL_TABLE *symb_tmp = NULL;
	char copy_operand[20] = { 0, }, *ptr;
	int i = -1;

	strcpy(copy_operand, symbol);
	copy_operand[strlen(symbol) - 1] = 0;
	if(copy_operand[0] == '@' || copy_operand[0] == '#') {
		for(i = 0; i < 20 - 1; i++) {
			copy_operand[i] = copy_operand[i+1];
		}
	}
	ptr = strtok(copy_operand, ",");
	i = ptr[0] - 'A';
	symb_tmp = symb_table[i];

	while(symb_tmp) {
		if(!strcmp(symb_tmp->symbol, ptr)) {
			break;
		}
		else {
			symb_tmp = symb_tmp->link;
		}
	}
	return symb_tmp->LOCCTR;
}

void directive_objcode(const char *mnemonic, const char *operand, int *directive_code) {
	int i = 0, tmp_hex = 0, position;
	char copy_operand[100] = { 0, }, tmp_str[2] = { 0, };

	strcpy(copy_operand, operand);
	copy_operand[strlen(operand) - 1] = 0;
	if(!strcmp(mnemonic, "WORD") || !strcmp(mnemonic, "RESW") || !strcmp(mnemonic, "RESB") || !strcmp(mnemonic, "BASE") || !strcmp(mnemonic, "NOBASE") || !strcmp(mnemonic, "END")) {
		for(i = 0; i < 30; i++) 
			directive_code[0] = 0;
		return;
	}
	else if(!strcmp(mnemonic, "BYTE")) {
		if(copy_operand[0] == 'C') {
			for(i = 2; i < strlen(copy_operand) - 1; i++) {
				directive_code[i - 2] = (int)copy_operand[i];
			}
		}
		else if(copy_operand[0] == 'X') {
			for(i = 2; i < strlen(copy_operand) - 1; i++) {
				tmp_str[0] = copy_operand[i];
				directive_code[i - 2] = (int)strtol(tmp_str, NULL, 16);
			}
		}
	}
}
