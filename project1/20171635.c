#include "20171635.h"

bool exit_flag = false;
char *input_str; //command line input

int main() {
	FILE *fp = fopen("opcode.txt", "r");
	memory = (unsigned char*)calloc((MEMORY_SIZE), sizeof(unsigned char));

	init_table();
	read_opcode(fp);

	while(1) {
		input_str = (char*)calloc((MAX_INPUT_LEN), sizeof(char));
		printf("sicsim> ");
		
		fgets(input_str, MAX_INPUT_LEN, stdin);
		read_command(input_str);
		free(input_str);

		if(exit_flag == true) break; 
	}
	
	free_hash_table();
	free(memory);
	fclose(fp);
	return 0;
}
