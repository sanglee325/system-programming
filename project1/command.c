#include "20171635.h"

extern bool exit_flag;
extern HISTORY_LIST *history;

/***** function for command help *****/
void command_help() {
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
void command_dir() {
	DIR *dp = NULL;
	struct dirent *entry = NULL;
	struct stat buf;

	dp = opendir(".");
	if(dp == NULL){
			printf("ERROR: directory open failure\n");
			return;
	}
	
	entry = readdir(dp);
	while((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &buf);
		
		//directory type file
		if(S_ISDIR(buf.st_mode)) {
			printf("%s/  ", entry->d_name);
		}
		else if(S_ISREG(buf.st_mode)) {
			//executable file
			if((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXOTH)) {
				printf("%s*  ", entry->d_name);
			}
			//regular file
			else {
				printf("%s  ", entry->d_name);
			}
		}
	}
	printf("\n");

	closedir(dp);
}

/***** function for command quit *****/
void command_quit() {
	NODE *target, *temp;

	exit_flag = true;
	if(history != NULL) {
		target = history->head;
		temp = target->link; 
		while(1) {
			temp = target->link;
			free(target);
			if(target->link == NULL) break;
			target = temp;
		}
	}
}

/***** function for command history *****/
void command_history(){
	NODE *target;

	target = history->head;
	while(1){
		printf("%-5d%s", target->num, target->str);
		if(target->link == NULL) {
			break;
		}
		else {
			target = target->link;
		}
	}	

}
