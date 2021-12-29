#include <stdio.h>
#include <string.h>	// strlen, strcmp
#include <ctype.h>	// tolower

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8

enum player{white, black};
const char *pieces = "kqrbn";

void display(char[][FILES]);
void askMove(int, char *);
int validateInput(char *);
int validateCastling(char *, int);
int validatePawnMove(char *, int);
int validatePieceMove(char *, int);
int isSquare(char *);
int canMove(char[][FILES], int, char *, int);
int canPawnMove(char[][FILES], int, char *);
int canPawnCapture(char[][FILES], int, char *);
int canPieceMove(char[][FILES], int, char *);
int canPieceCapture(char[][FILES], int, char *);
int canKingsideCastle(char[][FILES], int, char *);
int canQueensideCastle(char[][FILES], int, char *);

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
	int command;
	int moves = 1;

	while (isPlaying) {
		display(board);
		askMove(turn, input);

		if (!strcmp(input, "quit")) {
			isPlaying = 0;
		} else if((command = validateInput(input))) {
			if (command == 3 || command == 4) {
				input[0] = toupper(input[0]);
			}
			if (canMove(board, turn, input, command)) {
				printf("%d.%s%s\n", moves, turn ? ".." : "", input);
			} else {
				printf("Illegal move.\n");
				continue;
			}
		} else {
			printf("Invalid input.\n");
			continue;
		}

		if (turn) {
			moves++;
		}
		turn = ++turn % 2;
	}

	return 0;
}

void display(char board[][FILES]) {
	printf("---------------\n");
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

	for (int i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	}

	if ((c = str[0]) == '0' || c == 'o') {	// "o0" - castling
		return validateCastling(str, len);
	} else if (c >= 'a' && c <= 'h') {	// "abcdefgh" - pawn move
		return validatePawnMove(str, len);
	} else {
		i = 0;
		while (pieces[i]) {
			if (c == pieces[i++]) {
				return validatePieceMove(str, len);
			}
		}
	}	

	return 0;
}

int validatePawnMove(char *str, int len) {
	if (len == 2 && isSquare(str)) {
		return 1;	// pawn move
	} else if (len == 4 && str[1] == 'x' && isSquare(&str[2])) {
		return 2;	// pawn capture
	}
	return 0;
}

int validatePieceMove(char *str, int len) {
	if ((len == 3 && isSquare(&str[1])) || 
			(len == 4 && isSquare(&str[2]) &&
			((str[1] >= 'a' && str[1] <= 'h') ||
			(str[1] >= '1' && str[1] <= '8')))) {
		return 3;	// piece move
	} else if ((str[1] == 'x' && len == 4 && isSquare(&str[2])) ||
			(str[2] == 'x' && len == 5 && isSquare(&str[3]) &&
			((str[1] >= 'a' && str[1] <= 'h') ||
			(str[1] >= '1' && str[1] <= '8')))) {
		return 4;	// piece capture
	}
	return 0;
}

int validateCastling(char *str, int len) {
	if (!strcmp("o-o", str) || !strcmp("0-0", str)) {
		return 5;	// kingside castle
	} else if (!strcmp("o-o-o", str) || !strcmp("0-0-0", str)) {
		return 6;	// queenside castle
	}
	return 0;
}

int isSquare(char *str) {
	return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8';
}

int canMove(char board[][FILES], int side, char *input, int command) {
	switch(command){
		case 1:	return canPawnMove(board, side, input);
		case 2:	return canPawnCapture(board, side, input);
		case 3:	return canPieceMove(board, side, input);
		case 4:	return canPieceCapture(board, side, input);
		case 5:	return canKingsideCastle(board, side, input);
		case 6:	return canQueensideCastle(board, side, input);
		default:
				return 0;
	}
}

int getRow(char c) {
	return 8 - (c - '0');
}

int getCol(char c) {
	return c - 'a';
}

int canPawnMove(char board[][FILES], int side, char *input) {
	int row = getRow(input[1]);
	int col = getCol(input[0]);

	if (board[row][col] == '.') {
		if (side == white) { 
			if (board[row+1][col] == 'p' || 
					(row == 4 && board[row+1][col] == '.' && 
					board[6][col] == 'p')) {
				return 1;
			}
		} else {
			if (board[row-1][col] == 'P' || 
					(row == 3 && board[row-1][col] == '.' && 
					board[1][col] == 'P')) {
				return 1;
			}
		}
	}

	return 0;
}

int canPawnCapture(char board[][FILES], int side, char *input) {
	return 0;
}

int canPieceMove(char board[][FILES], int side, char *input) {
	return 0;
}

int canPieceCapture(char board[][FILES], int side, char *input) {
	return 0;
}

int canKingsideCastle(char board[][FILES], int side, char *input) {
	return 0;
}

int canQueensideCastle(char board[][FILES], int side, char *input) {
	return 0;
}
