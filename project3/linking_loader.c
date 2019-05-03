#include "20171635.h"

/*----------------------------------------------------------*/
/* function	: command_progaddr								*/
/* object	: sets the start address of where to load prog	*/
/* return	: none											*/
/*----------------------------------------------------------*/
void command_progaddr(int address) {
	progaddr = address;
	printf("Program starting address set to 0x%x.\n", address);
}

/*----------------------------------------------------------*/
/* function	: command_loader								*/
/* object	: loads program on the memory					*/
/* return	: true: load success/false: load fail			*/
/*----------------------------------------------------------*/
bool command_loader(int file_num, char** input) {
	char **filename;
	int i = 0;
	bool flag_pass1 = true, flag_pass2 = true;
	ESTAB *extern_symtab = NULL;

	filename = (char**)malloc(sizeof(char*) * file_num);
	for(i = 0; i < file_num; i++) {
		filename[i] = (char*)malloc(sizeof(char) * 50);
		strcpy(filename[i], input[i+1]);
	}

	extern_symtab = (ESTAB*)calloc(file_num, sizeof(ESTAB));

	flag_pass1 = linking_loader_pass1(progaddr, file_num, filename, extern_symtab);
	if(flag_pass1) {
		flag_pass2 = linking_loader_pass2(progaddr, file_num, filename, extern_symtab);
	}
	else {
		printf("ERROR: pass1 FAILED\n");
		return false;
	}

	if(flag_pass2) {
		return true;
	}
	else {
		printf("ERROR: pass2 FAILED\n");
		return false;
	}	

}


