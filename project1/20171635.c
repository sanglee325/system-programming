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

