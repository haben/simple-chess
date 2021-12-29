#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8

enum player{white, black};
const char *pieces = "kqrbn";

void display(char[][FILES]);
void askMove(int, char[MAX_CHAR]);
int validateInput(char[MAX_CHAR]);
int validateCastling(char[MAX_CHAR]);
int validatePawnMove(char[MAX_CHAR]);
int validatePieceMove(char[MAX_CHAR]);


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
	int isPlaying = 1;

	while (isPlaying) {
		display(board);
		askMove(turn, input);

		if (!strcmp(input, "quit")) {
			isPlaying = 0;
		} else {
			printf("%s!\n", validateInput(input) ? "PASS" : "FAIL");
			printf("---------------\n");
		}

		turn = ++turn % 2;
	}

	return 0;
}

void display(char board[][FILES]) {
	for (int i = 0; i < RANKS; i++) {
		for (int j = 0; j < FILES; j++) {
			printf("%c ", board[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void askMove(int turn, char *reply) {
	printf("%s to move: ", turn ? "Black" : "White");
	scanf("%s", reply);
	printf("\n");
}

int validateInput(char *str) {
	char c;
	int len = strlen(str);
	int i = 0;

	// immediately reject if input is too short
	if (len < 2) {
		return 0;
	} 

	c = str[0];
	if (isalpha(c)) {
		c = tolower(c);
	}

	if (c == '0' || c == 'o') {	// "o0" - castling
		return validateCastling(str);
	} else if (c >= 'a' && c <= 'h') {	// "abcdefgh" - pawn move
		return validatePawnMove(str);
	} else {
		while (pieces[i]) {
			if (c == pieces[i++]) {
				return validatePieceMove(str);
			}
		}
	}	

	return 0;
}

int validatePawnMove(char *str) {
	return 1;
}

int validatePieceMove(char *str) {
	return 1;
}

int validateCastling(char *str) {
	return 1;
}
