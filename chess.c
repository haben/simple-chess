
#include <stdio.h>

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8


enum player{white, black};

void display(char[][FILES]);
void askMove(int, char[MAX_CHAR]);

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

	int turn = white;
	char input[MAX_CHAR];

	display(board);
	askMove(turn, input);

	return 0;
}

void display(char board[][FILES]) {
	for (int i = 0; i < RANKS; i++) {
		for (int j = 0; j < FILES; j++)
			printf("%c ", board[i][j]);
		printf("\n");
	}
	printf("\n");
}

void askMove(int turn, char *reply) {
	printf("%s to move: ", turn ? "Black" : "White");
	scanf("%s", reply);
	printf("\n");
}
