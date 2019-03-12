#include "20171635.h"

bool exit_flag = false;
char input_str[MAX_INPUT_LEN]; //command line input
char **word; //tokenized command

int main() {
	FILE *fp = fopen("opcode.txt", "r");

	while(1) {
		printf("sicsim> ");
		
		fgets(input_str, sizeof(input_str), stdin);
		read_command(input_str);

		if(exit_flag == true) break; 
	}

	fclose(fp);
	return 0;
}

void read_command(char *input_str) {
	int idx = 0, word_num = 0;
	char *command;

	for(int i = 0; i < strlen(input_str); i++)
		if(input_str[i] == ' ')
			word_num++;
	word_num++;

	word = (char**)malloc(sizeof(char*) * word_num);

	if(word_num > 1)
		word[idx] = strtok(input_str, " ");
	else
		word[idx] = strtok(input_str, "\n");
	idx++;

	while(word[idx] = strtok(NULL, " ")) idx++;
	word[word_num] = strtok(NULL, "\n"); //change \n to \0


	command = word[0];

	//exception for \n
	if(input_str[0] == '\n') {
		printf("ERROR: Input command\n");
		return;
	}
	//command about help
	if(!strcmp(command, "help") || !strcmp(command, "h")) {
		command_help();
		return;
	}
	//command about dir
	if(!strcmp(command, "dir") || !strcmp(command, "d")) {
		command_dir();
		return;
	}
	//command about quit
	if(!strcmp(command, "quit") || !strcmp(command, "q")) {
		command_quit();
		return;
	}
	//command about history
	if(!strcmp(command, "history") || !strcmp(command, "hi")) {
		command_history();
		return;
	}

	else {
		printf("ERROR: Unvaild command\n");
		return;
	}
}		
