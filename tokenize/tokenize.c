#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char input[50] = "history dir  help\n";
char **word;
void tokenize(int word_num);

int main() {
	int count_word = 0;

	printf("%s", input);

	for(int i = 0; i < strlen(input); i++) {
		if(input[i] == ' ') {
			count_word++;
		}
	}

	tokenize(count_word + 1); 
	return 0;
}

void tokenize(int word_num) {
	int i = 0;
	word = (char**)malloc(sizeof(char*) * word_num);

	word[i] = strtok(input, " ");
	i++;

	printf("%s\n", word[0]); 

	while(word[i] = strtok(NULL, " ")) {
		printf("while\n");
		printf( "%s\n", word[i]);
		i++;
	}

	for(i = 0; i < word_num; i++){
		printf("word[%d]: %s\n", i, word[i]);
	}
}
