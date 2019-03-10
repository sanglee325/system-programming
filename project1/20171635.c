#include "20171635.h"

bool exit_flag = false;
int main(){
	FILE *fp = fopen("opcode.txt", "r");
	char input_str[MAX_INPUT_LEN],
		 *command;

	while(1){
		printf("sicsim> ");
		//input string needs to be tokenized
		fgets(input_str, sizeof(input_str), stdin);

		/* * * * * * * * * * * * * *
		 * TODO: \n input must run *
		 * runs segmentation fault *
		 * * * * * * * * * * * * * */

	 	command = strtok(input_str, " ");
		command = strtok(command, "\n");

		read_command(command);
		if(exit_flag == true) break; 
	}

	fclose(fp);
	return 0;
}

void read_command(char *command){
	//command about help
	if(!strcmp(command, "help") || !strcmp(command, "h")){
		command_help();
	}
	if(!strcmp(command, "dir") || !strcmp(command, "d")){
		command_dir();
	}
	if(!strcmp(command, "quit") || !strcmp(command, "q")){
		command_quit();
	}
	if(!strcmp(command, "history") || !strcmp(command, "hi")){
		command_history();
	}

}		
