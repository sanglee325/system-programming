#include "20171635.h"

bool command_assemble(char *filename) {
	FILE *file_asm, *file_lst, *file_obj;
	char *input_asm, *end_of_file;

	file_asm = fopen(filename, "r");
	if(file_asm == NULL) return false;

	while(1) {
		input_asm = (char*)malloc(MAX_INPUT_LEN * sizeof(char));
		end_of_file = fgets(input_asm, MAX_INPUT_LEN, file_asm);
		if(end_of_file == NULL) break; 
	}
	fclose(file_asm);	

	
}
