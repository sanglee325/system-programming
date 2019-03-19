#include "20171635.h"

extern char input_str[MAX_INPUT_LEN];
HISTORY_LIST *history;
int history_num = 0;

void read_command(char *input_str) {
	int i = 0, j = 0;
	int idx = 0, word_num = 0;
	int start = 0, end = 0, dump_idx, token_idx = 0;
	char *command, *ptr, *error = 0;
	char tokenize[100][101] = { 0 };
	bool valid = true, dump_valid = false;
	int delimiter = 0, delimiter_idx[100], word_end[100];
	int address = { 0 }, value = 0;
	bool  word[101] = { false }; //tokenized command
	NODE *data, *prev_node, *temp;

	//create history list node
	data = (NODE*)malloc(sizeof(NODE));
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
	//printf("delimiter ',': %d\n", delimiter);

	for(i = 1; i < strlen(input_str); i++) {
		if(word[i-1] == true && word[i] == false) {
			word_num++;
			if(word_num == 1) dump_idx = i;
		}
	}
	//printf("word_num: %d\n", word_num);

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
		printf("ERROR: Input command\n");
		valid = false;
	}
	//command about help
	else if(!strcmp(command, "help") || !strcmp(command, "h")) {
		if(word_num != 1)
			valid = false;
		else
			command_help();
	}
	//command about dir
	else if(!strcmp(command, "dir") || !strcmp(command, "d")) {
		if(word_num != 1)
			valid = false;
		else
			command_dir();
	}
	//command about quit
	else if(!strcmp(command, "quit") || !strcmp(command, "q")) {
		if(word_num != 1)
			valid = false;
		else
			command_quit();
	}
	//command about history
	else if(!strcmp(command, "history") || !strcmp(command, "hi")) {
		if(word_num != 1)
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
		if(word_num != 3) {
			valid = false;
		}
		else if(word_num == 3) {
			if(word_end[1] < delimiter_idx[0] && delimiter_idx[0] < word_end[2]) {
				address = (int)strtol(tokenize[1], &error, 16);
				if(*error) valid = false;
				value = (int)strtol(tokenize[2], &error, 16);
				if(*error) valid = false;
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
		if(word_num != 4) {
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
				if(valid && (0 <= start && start <= 0xFFFFF) &&
						(0 <= end && end <= 0xFFFFF) && (start <= end)) {
					command_fill(start, end, value);
				}
			}
			else {
				valid = false;
			}

		}
	}
	//command about reset
	else if(!strcmp(command, "reset")) {
		free(memory);
		memory = (unsigned char*)calloc(MEMORY_SIZE, sizeof(unsigned char));
	}
	//command about opcode mnemonic
	else if(!strcmp(command, "opcode")) {
	}
	//command about opcodelist
	else if(!strcmp(command, "opcodelist")) {
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
		printf("ERROR: Unvaild command\n");
	}
}
