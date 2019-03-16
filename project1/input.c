#include "20171635.h"

extern char input_str[MAX_INPUT_LEN];
HISTORY_LIST *history;
int history_num = 0;

void read_command(char *input_str) {
	int idx = 0, word_num = 0;
	char *command, *ptr;
	bool valid = true;
	char **word; //tokenized command
	NODE *data, *prev_node, *temp;

	data = (NODE*)malloc(sizeof(NODE));
	history_num++;
	data->num = history_num;
	strcpy(data->str, input_str);

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

	for(int i = 0; i <strlen(input_str); i++)
		if(!('!' <= input_str[i] && input_str[i] <= '~'))
			input_str[i] = ' ' ;
	for(int i = 1; i < strlen(input_str); i++)
		if(input_str[i-1] != ' ' && (input_str[i] == ' ' || input_str[i] == '\n'))
			word_num++;
	
	if(word_num == 0) word_num = 1;
	word = (char**)malloc(sizeof(char*) * word_num);
	for(int i = 0; i < word_num; i++) {
		word[i] = (char*)malloc(sizeof(char) * 100);
	}

	if(word_num >= 1)
		ptr = strtok(input_str, " ");
	else
		ptr = strtok(input_str, "\n");
	idx++;

	while(ptr = strtok(NULL, " ")){
		strcpy(word[idx], ptr);
		idx++;
	}
	word[word_num] = strtok(NULL, "\n"); //change \n to \0


	command = word[0];

	//exception for \n
	if(input_str[0] == '\n') {
		printf("ERROR: Input command\n");
		valid = false;
	}
	//command about help
	else if(!strcmp(command, "help") || !strcmp(command, "h")) {
		command_help();
	}
	//command about dir
	else if(!strcmp(command, "dir") || !strcmp(command, "d")) {
		command_dir();
	}
	//command about quit
	else if(!strcmp(command, "quit") || !strcmp(command, "q")) {
		command_quit();
	}
	//command about history
	else if(!strcmp(command, "history") || !strcmp(command, "hi")) {
		command_history();
	}
	else {
		printf("ERROR: Unvaild command\n");
		valid = false;
	}

	for(int i = 0; i < word_num; i++){
		free(word[i]);
	}
	free(word);

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
	}

}		
