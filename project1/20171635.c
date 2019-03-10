#include "20171635.h"

bool exit_flag = false;
int main() {
	FILE *fp = fopen("opcode.txt", "r");
	char input_str[MAX_INPUT_LEN];

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
	//command about invalid inputs
	char *command, copy_str[MAX_INPUT_LEN];
	
	strcpy(copy_str, input_str);// temporary save

	command = strtok(input_str, " ");

	if(!strcmp(command, "\n")) {
		printf("ERROR: Input command\n");
		return;
	}

	command = strtok(command, "\n");

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
