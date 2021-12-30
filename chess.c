#include <stdio.h>
#include <string.h>	// strlen, strcmp

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8

const char *pieces = "KQRBN";
enum player{white, black};

void display(char [][FILES]);
void askMove(int, char *);
int validateInput(char *);
int validateCastling(char *, int);
int validatePawnMove(char *, int);
int validatePieceMove(char *, int);
int isSquare(char *);
int isAlgebraic(char);
int isFile(char);
int isRank(char);
char isPawnMovingTwo(char [][FILES], int, char *, int);
int canMove(char[][FILES], int, char *, int, char *);
int getRow(char);
int getColumn(char);
char *getTargetSquare(char[][FILES], char *);
void makeMove(char[][MAX_CHAR], char *, char *);
char *getMovingPawn(char[][FILES], int, char *);
char *getCapturingPawn(char[][FILES], int, char *, char *);
char *getMovingPiece(char[][FILES], int, char *);
char *canPieceMove(char[][FILES], char, int, int, int, int);
char *findPiece(char[][FILES], char, int, int);


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
	char enPassant[] = {0, 0};

	while (isPlaying) {
		display(board);
		askMove(turn, input);

		if (!strcmp(input, "quit")) {
			isPlaying = 0;
		} else if((command = validateInput(input))) {
			if (canMove(board, turn, input, command, enPassant)) {
				printf("%d.%s%s\n", moves, turn ? ".." : "", input);
				enPassant[(turn+1)%2] = 0;
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

	if ((c = str[0]) == '0' || c == 'o' || c == 'O') {	// "oO0" - castling
		return validateCastling(str, len);
	} else if (isFile(c)) {	// "abcdefgh" - pawn move
		return validatePawnMove(str, len);
	} else {	// "NBRQK" - piece move
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
	if (isSquare(&str[len-2])) {
		if (len == 3 || (len == 4 && isAlgebraic(str[1])) ||
				(len == 5 && isRank(str[1]) && isFile(str[2]))) {
			return 3;	// piece move
		} else if ((str[1] == 'x' && len == 4) ||
				(str[2] == 'x' && len == 5 && isAlgebraic(str[1])) ||
				(str[3] == 'x' && len == 6 && isRank(str[1]) && 
				isFile(str[2]))) {
			return 4;	// piece capture
		}
	}
	return 0;
}

int validateCastling(char *str, int len) {
	if (!strcmp("o-o", str) || !strcmp("0-0", str) || !strcmp("O-O", str)) {
		return 5;	// kingside castle
	} else if (!strcmp("o-o-o", str) || !strcmp("0-0-0", str) || 
			!strcmp("O-O-O", str)) {
		return 6;	// queenside castle
	}
	return 0;
}

int isSquare(char *str) {
	return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8';
}

int isAlgebraic(char c) {
	return (c >= 'a' && c <= 'h') || (c >= '1' && c<= '8');
}

int isFile(char c) {
	return c >= 'a' && c <= 'h';
}

int isRank(char c) {
	return c >= '1' && c <= '8';
}

// returns rank of pawn if it moves two squares
char isPawnMovingTwo(char board[][FILES], int side, char *input, int len) {
	int file = getColumn(input[len-2]);

	if ((side == white && input[len-1] == '4' && 
			board[5][file] == '.' && board[6][file] == 'p') ||
			(side == black && input[len-1] == '5' && 
			board[2][file] == '.' && board[1][file] == 'P')) {
		return file + 'a';
	}
	return 0;
}

int canMove(char board[][FILES], int side, char *input, int command, 
		char *enPassant) {
	char *to, *from;
	int len = strlen(input);

	switch(command) {
		case 1:	from = getMovingPawn(board, side, input);
				// if pawn moves 2 spaces, enPassant is true for this side
				enPassant[side] = isPawnMovingTwo(board, side, input, len);
				break;
		case 2:	from = getCapturingPawn(board, side, input, enPassant);
				break;
		case 3:	from = getMovingPiece(board, side, input);
				break;
		case 4:	return 1;
		case 5:	return 1;
		case 6:	return 1;
		default:
				return 0;
	}

	if (command >= 1 && command <= 3) {
		if (!from) {
			return 0;
		} else {
			to = getTargetSquare(board, &input[len-2]);
			makeMove(board, to, from);
			return 1;
		}
	}
}

int getRow(char c) {
	return 8 - (c - '0');
}

int getColumn(char c) {
	return c - 'a';
}

char *getTargetSquare(char board[][MAX_CHAR], char *input) {
	int len = strlen(input);
	return &board[getRow(input[len-1])][getColumn(input[len-2])];
}

void makeMove(char board[][MAX_CHAR], char *to, char *from) {
	char ch = *from;

	*from = '.';
	*to = ch;
}

int isEnemy(char c, int side) {
	return (side == white && c >= 'A' && c < 'Z') ||
		(side == black && c >= 'a' && c < 'z');
}

char *getMovingPawn(char board[][FILES], int side, char *input) {
	int row = getRow(input[1]);
	int col = getColumn(input[0]);

	if (board[row][col] == '.') {
		if (side == white) { 
			if (board[row+1][col] == 'p') {
			   return &board[row+1][col];
			} else if (row == 4 && board[row+1][col] == '.' && 
					board[6][col] == 'p') {
			   return &board[6][col];
			}
		} else {
			if (board[row-1][col] == 'P') {
				return &board[row-1][col];
			} else if (row == 3 && board[row-1][col] == '.' && 
					board[1][col] == 'P') {
				return &board[1][col];
			}
		}
	}

	return 0;
}

char *getCapturingPawn(char board[][FILES], int side, char *input, 
		char *enPassant) {
	int len = strlen(input);
	int row = getRow(input[len-1]);
	int col = getColumn(input[len-2]);
	char target = board[row][col];
	int file = getColumn(input[0]);

	if (file == col - 1 || file == col + 1) {
		if (target != '.' && isEnemy(target, side)) {	// normal capture
			if (side == white && board[row+1][file] == 'p') {
				return &board[row+1][file];
			} else if (side == black && board[row-1][file] == 'P') {
				return &board[row-1][file];
			}
		} else if (enPassant[(side+1)%2] == input[len-2]) {	// en passant
			if (side == white && isEnemy(board[row+1][col], white) &&
					board[row+1][file] == 'p') {
				board[row+1][col] = '.';
				return &board[row+1][file];
			} else if (side == black && isEnemy(board[row-1][col], black) &&
					board[row-1][file] == 'P') {
				board[row-1][col] = '.';
				return &board[row-1][file];
			}
		}
	}

	return 0;
}

int matchPieceRow(char board[][FILES], char piece, int col) {
	int count = 0;
	int row = -1;

	for (int i = 0; i < RANKS; i++) {
		if (board[i][col] == piece) {
			if (!count) {
				row = i;
				count++;
			} else {
				return -1;
			}
		}
	}
	return row >= 0 && row <= 7;
}

int matchPieceColumn(char board[][FILES], char piece, int row) {
	int count = 0;
	int col = -1;

	for (int i = 0; i < FILES; i++) {
		if (board[row][i] == piece) {
			if (!count) {
				col = i;
				count++;
			} else {
				return -1;
			}
		}
	}
	return col >= 0 && col <= 7;
}

char *getMovingPiece(char board[][FILES], int side, char *input) {
	int len = strlen(input);
	int row = getRow(input[len-1]);
	int col = getColumn(input[len-2]);
	char piece = input[0];
	int pRow = -1;
	int pCol = -1;
	int extraChar = 0;

	if (board[row][col] == '.') {
		if (len == 5) {
			pRow = getRow(input[1]);
			pCol = getColumn(input[2]);
			if (board[pRow][pCol] != piece) {
				return 0;
			}
			return canPieceMove(board, piece, pRow, pCol, row, col);
		} else if (len == 4) {
			if	(input[1] >= 'a' && input[1] <= 'h') {
				pCol = getColumn(input[1]);
				pRow = matchPieceRow(board, piece, pCol);
			} else {
				pRow = getRow(input[1]);
				pCol = matchPieceColumn(board, piece, pRow);
			}
			if (pRow == -1 || pCol == -1) {
				return 0;
			}
			return canPieceMove(board, piece, pRow, pCol, row, col);
		} else {
			return findPiece(board, piece, row, col);
		}
	}

	return 0;
}

char *canPieceMove(char board[][FILES], char piece, int row0, int col0, 
		int row1, int col1) {
	return 0;
}

char *findPiece(char board[][FILES], char piece, int row, int col) {
	return 0;
}
