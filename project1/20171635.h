#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_INPUT_LEN 100
#define MAX_CMD_LEN 10

void read_command(char *command);

/*------ shell_command-----*/
void command_help();
void command_dir();
void command_quit();
void command_history();

