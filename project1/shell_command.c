#include "20171635.h"

extern bool exit_flag;

void command_help(){
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
}
void command_dir(){
}
void command_quit(){
	exit_flag = true;
}
void command_history(){
}
