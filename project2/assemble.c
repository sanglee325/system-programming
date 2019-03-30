#include "20171635.h"

char origin[500];
bool command_assemble(char *filename) {
	FILE *file_asm;
	int i, j, idx, token_idx, delimiter = 0, word_num = 2;
	int program_len = 0;
	char **tokenize, tmp_name[500];
	bool word[MAX_INPUT_LEN] = { false, };

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
		assemble_pass1(file_asm, &program_len);
		fclose(file_asm);	
	}
	else {
		printf("Invalid File type!\n");
		for(i = 0; i < word_num; i++)
			free(tokenize[i]);
		free(tokenize);
		return false;
	}
}

void assemble_pass1(FILE* file_asm, int *program_len) {
	FILE *file_inter, *file_lst, *file_obj;
	char *input_asm, *end_of_file;
	int start_address, LOCCTR = 0, line_num = 5, operand = 0;

	file_inter = fopen("inter.asm", "w");

	while(1) {
		input_asm = (char*)malloc(MAX_INPUT_LEN * sizeof(char));
		fgets(input_asm, MAX_INPUT_LEN, file_asm);
		
		if(!check_comment(input_asm)){
			break;
		}

	}



}

bool check_comment(const char* input) {
	int i = 0;
	while(input[i] == ' ' || input[i] == '\t' || input[i] == '\n') i++;

	if(input[i] == '.') {
		return true;
	}
	else {
		return false;
	}
}
