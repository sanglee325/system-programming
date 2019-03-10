#include "20171635.h"

extern bool exit_flag;

/***** function for command help *****/
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

/***** function for command dir *****/
void command_dir(){
	DIR *dp = NULL;
	struct dirent *entry = NULL;
	struct stat buf;

	dp = opendir(".");
	if(dp == NULL){
			printf("ERROR: directory open failure\n");
			return;
	}
	
	entry = readdir(dp);
	while((entry=readdir(dp)) != NULL){
		lstat(entry->d_name, &buf);
		
		if(S_ISDIR(buf.st_mode))
			printf("%s/  ", entry->d_name);
		else if(S_ISREG(buf.st_mode))
			printf("%s  ", entry->d_name);
	}
	printf("\n");

	closedir(dp);
}

/***** function for command quit *****/
void command_quit(){
		exit_flag = true;
}

/***** function for command history *****/
void command_history(){
}

