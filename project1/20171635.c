#include "20171635.h"

void read_cmd(char *cmd);
int main(){
	char cmd[20];

	while(1){
		printf("sicsim> ");
		scanf("%s", cmd);
		
	}

	return 0;
}
void read_cmd(char *cmd){
	typedef enum CMD{
		help, dir, quit, history,
		dump, edit, fill, reset,
		opcode, opcodelist
	};

	CMD command;
	
}		
