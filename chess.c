#include <stdio.h>
#include <string.h>	// strlen, strcmp

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8

const char *pieces = "KQRBN";
const int diagonalMoves[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
const int orthogonalMoves[4][2] = {{-1, 0}, {0, -1}, {0, 1}, {1, 0}};
const int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2},
	{1, 2}, {2, -1}, {2, 1}};
const int kingMoves[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1},
	{1, -1}, {1, 0}, {1, 1}};

enum player{ WHITE, BLACK };

void display(char [][FILES]);
void askMove(int, char *);
int validateInput(char *);
int validateCastling(char *, int);
int validatePawnMove(char *, int);
int validatePieceMove(char *, int);
int isFile(char);
int isRank(char);
int isAlgebraic(char);
int isSquare(char *);
int isInBounds(int, int);
char findPawnMovingTwo(char [][FILES], int, char *, int);
int canMove(char[][FILES], int, char *, int, char *);
int getRow(char);
int getColumn(char);
char *getTargetSquare(char[][FILES], char *);
void makeMove(char[][MAX_CHAR], char *, char *);
char *canPieceMove(char[][MAX_CHAR], char, int, int, int, int);
int canKingOrKnightMove(char, int, int);
int canMoveDiagonally(char[][MAX_CHAR], int, int, int, int);
int canMoveStraight(char[][MAX_CHAR], int, int, int, int);
char *getMovingPawn(char[][FILES], int, char *);
char *getCapturingPawn(char[][FILES], int, char *, char *);
char *getMovingPiece(char[][FILES], int, char *);
char *getCapturingPiece(char[][FILES], int, char *);
char *findPiece(char[][FILES], char, int, int);
char *findFourWayPiece(char[][FILES], char, int, int, char);
char *findKingOrKnight(char[][FILES], char, int, int);


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

	int turn = WHITE;
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
	printf("+-----------------+\n");
	for (int i = 0; i < RANKS; i++) {
		printf("|");
		for (int j = 0; j < FILES; j++) {
			printf(" %c", board[i][j]);
		}
		printf(" | %d\n", 8 - i);
	}
	printf("+-----------------+\n ");
	for (int i = 0; i < FILES; i++) {
		printf(" %c", i + 'a');
	}
	printf("\n");
}

void askMove(int turn, char *reply) {
	printf("\n%s to move: ", turn ? "Black" : "White");
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

int isFile(char c) {
	return c >= 'a' && c <= 'h';
}

int isRank(char c) {
	return c >= '1' && c <= '8';
}

int isAlgebraic(char c) {
	return (c >= 'a' && c <= 'h') || (c >= '1' && c<= '8');
}

int isSquare(char *str) {
	return str[0] >= 'a' && str[0] <= 'h' && str[1] >= '1' && str[1] <= '8';
}

int isInBounds(int m, int n) {
	return m >= 0 && m <= 7 && n >= 0 && n <= 7;
}

// returns rank of pawn if it moves two squares
char findPawnMovingTwo(char board[][FILES], int side, char *input, int len) {
	int file = getColumn(input[len-2]);

	if ((side == WHITE && input[len-1] == '4' && 
			board[5][file] == '.' && board[6][file] == 'p') ||
			(side == BLACK && input[len-1] == '5' && 
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
				enPassant[side] = findPawnMovingTwo(board, side, input, len);
				break;
		case 2:	from = getCapturingPawn(board, side, input, enPassant);
				break;
		case 3:	from = getMovingPiece(board, side, input);
				break;
		case 4:	from = getCapturingPiece(board, side, input);
				break;
		case 5:	return 1;
		case 6:	return 1;
		default:
				return 0;
	}

	if (command >= 1 && command <= 4) {
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
	return (side == WHITE && c >= 'A' && c < 'Z') ||
		(side == BLACK && c >= 'a' && c < 'z');
}

char *getMovingPawn(char board[][FILES], int side, char *input) {
	int row = getRow(input[1]);
	int col = getColumn(input[0]);

	if (board[row][col] == '.') {
		if (side == WHITE) { 
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
			if (side == WHITE && board[row+1][file] == 'p') {
				return &board[row+1][file];
			} else if (side == BLACK && board[row-1][file] == 'P') {
				return &board[row-1][file];
			}
		} else if (enPassant[(side+1)%2] == input[len-2]) {	// en passant
			if (side == WHITE && isEnemy(board[row+1][col], WHITE) &&
					board[row+1][file] == 'p') {
				board[row+1][col] = '.';
				return &board[row+1][file];
			} else if (side == BLACK && isEnemy(board[row-1][col], BLACK) &&
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
	int found = 0;
	int col = -1;

	for (int i = 0; i < FILES; i++) {
		if (board[row][i] == piece) {
			if (!found) {
				col = i;
				found++;
			} else {
				return -1;
			}
		}
	}
	return col >= 0 && col <= 7;
}

char *getMovingPiece(char board[][FILES], int side, char *input) {
	int len = strlen(input);
	int row0 = -1;
	int col0 = -1;
	int row1 = getRow(input[len-1]);
	int col1 = getColumn(input[len-2]);
	char piece = input[0];

	if (board[row1][col1] == '.') {
		if (side == WHITE) {
			piece += 32;
		}

		if (len == 5) {
			row0 = getRow(input[1]);
			col0 = getColumn(input[2]);
			if (board[row0][col0] != piece) {
				printf("Square doesn't have piece indicated.\n");
				return 0;
			}
			return canPieceMove(board, piece, row0, col0, row1, col1);
		} else if (len == 4) {
			if	(isFile(input[1])) {
				col0 = getColumn(input[1]);
				row0 = matchPieceRow(board, piece, col0);
			} else {
				row0 = getRow(input[1]);
				col0 = matchPieceColumn(board, piece, row0);
			}
			if (row0 == -1 || col0 == -1) {
				printf("No piece matching rank or file given.\n");
				return 0;
			}
			return canPieceMove(board, piece, row0, col0, row1, col1);
		} else {
			return findPiece(board, piece, row1, col1);
		}
	}

	return 0;
}

char *canPieceMove(char board[][FILES], char piece, int row0, int col0, 
		int row1, int col1) {
	switch(piece) {
		case 'n':
		case 'N':	
		case 'k':
		case 'K':	if (canKingOrKnightMove(piece, row1-row0, col1-col0)) {
						return &board[row0][col0];
					}
		case 'b':
		case 'B':
		case 'r':
		case 'R':	
		case 'q':
		case 'Q':	if (canMoveStraight(board, row0, col0, row1, col1)) {
						return &board[row0][col0];
					}
	}

	return 0;
}

int canKingOrKnightMove(char piece, int rows, int cols) {
	for (int i = 0; i < 8; i++) {
		if (piece == 'n' || piece == 'N') {
			if (rows == knightMoves[i][0] && cols == knightMoves[i][1]) {
				return 1;
			}
		} else {
			if (rows == kingMoves[i][0] && cols == kingMoves[i][1]) {
				return 1;
			}
		}
	}
	printf("Piece cannot move there from current position.\n");
	return 0;
}

int canMoveDiagonally(char board[][FILES], int row0, int col0, int row1, 
		int col1) {
	int r = (row1 < row0) ? -1 : 1;
	int c = (col1 < col0) ? -1 : 1;
	int i = row0 + r;
	int j = col0 + c;

	for (; i != row1 && j != col1; i += r, j += c) {
		if (board[i][j] != '.') {
			return 0;
			printf("Piece is blocked from moving.\n");
		}
	}

	return 1;
}

int canMoveStraight(char board[][FILES], int row0, int col0, int row1, 
		int col1) {
	int r, c, i, j;
	
	r = (row1 - row0) ? ((row1 < row0) ? -1 : 1) : 0; 
	c = (col1 - col0) ? ((col1 < col0) ? -1 : 1) : 0; 

	for (i = row0 + r, j = col0 + c; i != row1 && j != col1; i += r, j += c) {
		if (board[i][j] != '.') {
			printf("Piece is blocked from moving.\n");
			return 0;
		}
	}

	return 1;
}

char *findPiece(char board[][FILES], char piece, int row, int col) {
	char *square = 0;

	switch (piece) {
		case 'n':
		case 'N':	return findKingOrKnight(board, piece, row, col);
		case 'b':
		case 'B':	return findFourWayPiece(board, piece, row, col, 'd');
		case 'r':
		case 'R':	return findFourWayPiece(board, piece, row, col, 'o');
		case 'q':
		case 'Q':	square = findFourWayPiece(board, piece, row, col, 'd');
					return square ? square : 
						findFourWayPiece(board, piece, row, col, 'o');
		case 'k':
		case 'K':	return findKingOrKnight(board, piece, row, col);
	}

	return 0;
}

char *findFourWayPiece (char board[][FILES], char piece, int row1, int col1, 
		char direction) {
	int row0, col0, r, c;
	int found = 0;
	char *square = 0;

	for (int i = 0; i < 4; i++) {
		r = (direction == 'd') ? diagonalMoves[i][0] : orthogonalMoves[i][0];
		c = (direction == 'd') ? diagonalMoves[i][1] : orthogonalMoves[i][1];

		row0 = row1 + r;
		col0 = col1 + c;

		while (isInBounds(row0, col0) &&
				(board[row0][col0] == piece || board[row0][col0] == '.')) {
			if (board[row0][col0] == piece) {
				if (!found++) {
					square = &board[row0][col0];
				} else {
					printf("Another piece found that can make that move.\n");
					return 0;
				}
			}
			row0 += r;
			col0 += c;
		}
	}

	return square;
}

char *findKingOrKnight(char board[][FILES], char piece, int row1, int col1) {
	int row0, col0, r, c;
	int found = 0;
	char *square = 0;

	for (int i = 0; i < 8; i++) {
		if (piece == 'n' || piece == 'N') {
			row0 = row1 + knightMoves[i][0];
			col0 = col1 + knightMoves[i][1];
		} else {
			row0 = row1 + kingMoves[i][0];
			col0 = col1 + kingMoves[i][1];
		}

		if (!isInBounds(row0, col0)) {
			continue;
		}
		if (board[row0][col0] == piece) {
			if (!found++) {
				square = &board[row0][col0];
			} else {
				printf("Another piece found that can make that move.\n");
				return 0;
			}
		}
	}

	return square;
}


char *getCapturingPiece(char board[][FILES], int side, char *input) {
	int len = strlen(input);
	int row0 = -1;
	int col0 = -1;
	int row1 = getRow(input[len-1]);
	int col1 = getColumn(input[len-2]);
	char piece = input[0];

	if (isEnemy(board[row1][col1], side)) {
		if (side == WHITE) {
			piece += 32;
		}

		if (len == 6) {
			row0 = getRow(input[1]);
			col0 = getColumn(input[2]);
			if (board[row0][col0] != piece) {
				printf("Square doesn't have piece indicated.\n");
				return 0;
			}
			return canPieceMove(board, piece, row0, col0, row1, col1);
		} else if (len == 5) {
			if	(isFile(input[1])) {
				col0 = getColumn(input[1]);
				row0 = matchPieceRow(board, piece, col0);
			} else {
				row0 = getRow(input[1]);
				col0 = matchPieceColumn(board, piece, row0);
			}
			if (row0 == -1 || col0 == -1) {
				printf("No piece matching rank or file given.\n");
				return 0;
			}
			return canPieceMove(board, piece, row0, col0, row1, col1);
		} else {
			return findPiece(board, piece, row1, col1);
		}
	}

	return 0;
}
