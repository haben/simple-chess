#include <stdio.h>

#define RANKS 8
#define FILES 8

void display(char[][FILES]);

int main() {
	char board[RANKS][FILES] = {
		{'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
		{'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
		{'.', '.', '.', '.', '.', '.', '.', '.'},
		{'.', '.', '.', '.', '.', '.', '.', '.'},
		{'.', '.', '.', '.', '.', '.', '.', '.'},
		{'.', '.', '.', '.', '.', '.', '.', '.'},
		{'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
		{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
	};

	display(board);

	return 0;
}

void display(char board[][FILES]) {
	for (int i = 0; i < RANKS; i++) {
		for (int j = 0; j < FILES; j++)
			printf("%c ", board[i][j]);
		printf("\n");
	}
}
