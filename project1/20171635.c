#include "20171635.h"

bool exit_flag = false;
int main() {
	FILE *fp = fopen("opcode.txt", "r");
	char input_str[MAX_INPUT_LEN],
		 *command;

	while(1) {
		printf("sicsim> ");
		//input string needs to be tokenized
		fgets(input_str, sizeof(input_str), stdin);

		/* * * * * * * * * * * * * *
		 * TODO: \n input must run *
		 * runs segmentation fault *
		 * * * * * * * * * * * * * */

	 	command = strtok(input_str, " ");
		
		read_command(command);
		if(exit_flag == true) break; 
	}

	fclose(fp);
	return 0;
}

void read_command(char *command) {
	//command about invalid inputs
	if(!strcmp(command, "\n")) {
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
