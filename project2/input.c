#include "20171635.h"

extern char *input_str;
HISTORY_LIST *history;
int history_num = 0;

/*----------------------------------------------------------*/
/* function	: read_command									*/
/* object	: read input from user, tokenize input			*/
/*			  calls function of given input 				*/
/* return	: none											*/
/*----------------------------------------------------------*/
void read_command(char *input_str) {
	int i = 0, j = 0;
	int idx = 0, word_num = 0;
	int start = 0, end = 0, token_idx = 0;
	char *command, *error = 0;
	char tokenize[MAX_INPUT_LEN][MAX_INPUT_LEN] = { 0 };
	bool valid = true, dump_valid = false, flag_type = true, flag_asm = true;
	int delimiter = 0, delimiter_idx[MAX_INPUT_LEN], word_end[MAX_INPUT_LEN];
	int address = 0, value = 0;
	bool word[MAX_INPUT_LEN] = { false }; //tokenized command
	HISTORY_NODE *data, *prev_node, *temp;

	//create history list node
	data = (HISTORY_NODE*)malloc(sizeof(HISTORY_NODE));
	history_num++;
	data->num = history_num;
	strcpy(data->str, input_str);

	//case of the first node
	if(history == NULL) {
		history = (HISTORY_LIST*)malloc(sizeof(HISTORY_LIST));
		history->head = data;
		history->tail = data;
		prev_node = NULL;
	}
	else {
		data->link = NULL;
		prev_node = history->tail;
		history->tail->link = data;
		history->tail = data;
	}

	//counting words
	j = 0;
	for(i = 0; i < strlen(input_str); i++) {
		if(('!' <= input_str[i] && input_str[i] <= '~')) {
			if(input_str[i] == ',') {
				delimiter++;
				delimiter_idx[j] = i;
				j++;
			}
			else
				word[i] = true;
		}
	}

	for(i = 1; i < strlen(input_str); i++) {
		if(word[i-1] == true && word[i] == false) {
			word_num++;
		}
	}

	//in case of no words
	if(word_num == 0) {
		tokenize[0][0] = '\n';
	}

	//tokenizing the input command
	j = 0;
	if(word_num > 0) {
		idx = 0;
		for(i = 0; i < strlen(input_str); i++){
			if(word[i] == true) {
				tokenize[idx][token_idx] = input_str[i];
				token_idx++;
			}
			if(word[i] == true && word[i+1] == false) { 
				word_end[j] = i; j++;
				idx++;
				token_idx = 0;
			}
			if(idx == word_num) break;
		}
	}
	command = tokenize[0];

	//exception for \n
	if(input_str[0] == '\n') {
		valid = false;
	}
	//command about help
	else if(!strcmp(command, "help") || !strcmp(command, "h")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_help();
	}
	//command about dir
	else if(!strcmp(command, "dir") || !strcmp(command, "d")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_dir();
	}
	//command about quit
	else if(!strcmp(command, "quit") || !strcmp(command, "q")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_quit();
	}
	//command about history
	else if(!strcmp(command, "history") || !strcmp(command, "hi")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_history();
	}
	//command about dump
	else if(!strcmp(command, "dump") || !strcmp(command, "du")) {
		if(word_num > 3) {
			valid = false;
		}
		else if(word_num == 1 && delimiter == 0) {
			start = -1; end = -1;
			dump_valid = true;
		}
		else if(word_num == 2 && delimiter == 0) {
			start = (int)strtol(tokenize[1], &error, 16);
			if(*error) valid = false;
			else if(valid && start >= 0) {
				end = -1;
				dump_valid = true;
			}
		}
		else if(word_num == 3 && delimiter == 1) {
			if(word_end[1] < delimiter_idx[0] && delimiter_idx[0] < word_end[2]) {
				start = (int)strtol(tokenize[1], &error, 16);
				if(*error) valid = false;
				end = (int)strtol(tokenize[2], &error, 16);
				if(*error) valid = false;
				if(valid) {
					if(start >= 0 && end >= 0 && end >= start) {
						dump_valid = true;
					}
				}
			}
		}
		else {
			valid = false;
		}

		if(valid) {
			if((start > 0xFFFFF) || (end > 0xFFFFF)) {
				valid = false;
			}
			if(!dump_valid) {
				valid = false;
			}
		}
		if(valid) command_dump(start, end);
	}
	//command about edit
	else if(!strcmp(command, "edit") || !strcmp(command, "e")) {
		if(word_num != 3 || delimiter != 1) {
			valid = false;
		}
		else if(word_num == 3) {
			if(word_end[1] < delimiter_idx[0] && delimiter_idx[0] < word_end[2]) {
				address = (int)strtol(tokenize[1], &error, 16);
				if(*error) valid = false;
				value = (int)strtol(tokenize[2], &error, 16);
				if(*error) valid = false;
				if(value < 0 || value > 0xFF) valid = false;
				if(valid && (0 <= address && address <= 0xFFFFF)) {
					command_edit(address, value);
				}
			}
			else {
				valid = false;
			}
		}
	}
	//command about fill
	else if(!strcmp(command, "fill") || !strcmp(command, "f")) {
		if(word_num != 4 || delimiter != 2) {
			valid = false;
		}
		else if(word_num == 4) {
			if((word_end[1] < delimiter_idx[0]) && (delimiter_idx[0] < word_end[2]) &&
					(word_end[2] < delimiter_idx[1]) && (delimiter_idx[1] < word_end[3])) {
				start = (int)strtol(tokenize[1], &error, 16);
				if(*error) valid = false;
				end = (int)strtol(tokenize[2], &error, 16);
				if(*error) valid = false;
				value = (int)strtol(tokenize[3], &error, 16);
				if(*error) valid = false;
				if(value < 0 || value > 0xFF) valid = false;
				if(valid) {
					if(start < 0 || start > 0xFFFFF)
						valid = false;
					if(end < 0 || end > 0xFFFFF)
						valid = false;
					if(start > end)
						valid = false;
				}
				if(valid) command_fill(start, end, value);
			}
			else {
				valid = false;
			}

		}
	}
	//command about reset
	else if(!strcmp(command, "reset")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_reset();
	}
	//command about opcode mnemonic
	else if(!strcmp(command, "opcode")) {
		if(word_num != 2 || delimiter > 0)
			valid = false;
		else
			valid = command_opcode(tokenize[1]);
	}
	//command about opcodelist
	else if(!strcmp(command, "opcodelist")) {
		if(word_num != 1 || delimiter > 0)
			valid = false;
		else
			command_opcodelist();
	}
	else if(!strcmp(command, "type")) {
		if(word_num != 2 || delimiter > 0)
			valid = false;
		else {
			flag_type = command_type(tokenize[1]);
			if(flag_type) { valid = true; }
			else { valid = false; }
		}
	}
	else if(!strcmp(command, "assemble")) {
		if(word_num != 2 || delimiter > 0)
			valid = false;
		else {
			flag_asm = command_assemble(tokenize[1]);
			if(flag_asm) { valid = true; }
			else { valid = false; }
		}
	}
	else {
		valid = false;
	}


	if(!valid) {
		if(prev_node == NULL) {
			history->head = NULL;	history->tail = NULL;
			free(history);
			history = NULL;
			free(data);
			history_num = 0;
		}
		else {
			temp = history->tail;
			prev_node->link = NULL;
			history->tail = prev_node;
			history_num--;
			free(temp);
		}
		if(flag_type != false && flag_asm != false) {
			printf("ERROR: Invaild command\n");
		}
	}
}

/*----------------------------------------------------------*/
/* function	: init_table									*/
/* object	: initialize the hash table						*/
/* return	: none											*/
/*----------------------------------------------------------*/
void init_table() {
	int i;
	for(i = 0; i < OPCODE_HASH_TABLE_SIZE; i++)
		table[i] = NULL;
	for(i = 0; i < 26; i++)
		symb_table[i] = NULL;
}

/*----------------------------------------------------------*/
/* function	: free_hash_table								*/
/* object	: free hash table when program is done			*/
/* return	: none											*/
/*----------------------------------------------------------*/
void free_hash_table() {
	int i;
	OPCODE_NODE *tmp_node, *target;
	
	for(i = 0; i < OPCODE_HASH_TABLE_SIZE; i++) {
		tmp_node = table[i];
		while(1) {
			if(tmp_node == NULL) {
				break;
			}
			else {
				if(tmp_node->link != NULL) {
					target = tmp_node;
					tmp_node = tmp_node->link;
					free(target);
				}
				else {
					target = tmp_node;
					tmp_node = tmp_node->link;
					free(target);
					break;
				}
			}
		}
	}
}

/*----------------------------------------------------------*/
/* function	: read_opcode									*/
/* object	: read opcode.txt and make opcode hash table	*/
/* return	: none											*/
/*----------------------------------------------------------*/
void read_opcode(FILE *fp) {
	OPCODE_NODE *node, *tmp_node;
	int i, token_idx = 0, word_num = 3, idx = 0, format_num = 0, sum_char = 0, hash_idx;
	char *end_of_file;
	char *tmp_str, **tokenize;
	bool word[MAX_INPUT_LEN] = { false };

	while(1) {
		tmp_str = (char*)calloc(MAX_INPUT_LEN, sizeof(char));
		end_of_file = fgets(tmp_str, MAX_INPUT_LEN, fp);
		if(end_of_file == NULL) {
			free(tmp_str);
			break;
		}
		
		//create and initialize node
		node = (OPCODE_NODE*)malloc(sizeof(OPCODE_NODE));
		node->link = NULL;
		for(i = 0; i < 5; i++) {
			node->format[i] = false;
		} 

		//create tokens and tokenize the word
		tokenize = (char**)malloc(sizeof(char*) * 3);
		for(i = 0; i < 3; i++)
			tokenize[i] = (char*)calloc(11, sizeof(char));

		for(i = 0; i < strlen(tmp_str); i++) {
			if(('!' <= tmp_str[i] && tmp_str[i] <= '~')) {
				word[i] = true;
			}
		}
		idx = 0;
		for(i = 0; i < strlen(tmp_str); i++){
			if(word[i] == true) {
				tokenize[idx][token_idx] = tmp_str[i];
				token_idx++;
			}
			if(word[i] == true && word[i+1] == false) { 
				idx++;
				token_idx = 0;
			}
			if(idx == word_num) break;
		} //tokenize words

		//find hash key
		sum_char = 0;
		for(i = 0; i < strlen(tokenize[1]); i++) {
			sum_char += (int)tokenize[1][i];
		}

		hash_idx = sum_char % OPCODE_HASH_TABLE_SIZE;

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

		if(table[hash_idx] == NULL) {
			table[hash_idx] = node;
		}
		else {
			tmp_node = table[hash_idx];
			while(1) {
				if(tmp_node->link == NULL) {
					tmp_node->link = node;
					break;
				}
				else {
					tmp_node = tmp_node->link;
				}
			}
		}

		free(tmp_str);
		//initialize variables for next loop
		for(i = 0; i < 3; i++)
			free(tokenize[i]);
		free(tokenize);
		for(i = 0; i < MAX_INPUT_LEN; i++)
			word[i] = false;
	}
}
