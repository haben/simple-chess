#include <stdio.h>
#include <string.h>	// strlen, strcmp

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8

const char *pieces = "KQRBN";
const int direction[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1},
	{-1, -1}, {-1, 0}, {-1, 1}};
const int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2},
	{1, 2}, {2, -1}, {2, 1}};

enum player{ WHITE, BLACK };

void display(char [][FILES]);
void askMove(int, char *);
int validateInput(char *);
int validatePawnMove(char *, int);
int validatePieceMove(char *, int);
int validateCastling(char *, int);
int isFile(char);
int isRank(char);
int isAlgebraic(char);
int isSquare(char *);
int isInBounds(int, int);
char findPawnMovingTwo(char [][FILES], int, char *);
int canMove(char[][FILES], int, char *, int, char *, int[][2]);
int canCastle(char[][FILES], int, int, int[][2]);
void trackCastle(char, int, int, int[][2]);
int getRow(char);
int getColumn(char);
char *getTargetSquare(char[][FILES], char *);
void makeMove(char[][MAX_CHAR], char *, char *);
int isEnemy(char, int);
char *canPieceMove(char[][MAX_CHAR], char, int, int, int, int);
int canMoveShortRanged(char, int, int);
int canMoveLongRanged(char[][MAX_CHAR], int, int, int, int);
char *getMovingPawn(char[][FILES], int, char *);
char *getCapturingPawn(char[][FILES], int, char *, char *);
int matchPiece(char[][FILES], char, int, int);
char *getPiece(char[][FILES], int, char *, int[][2], int);
char *findPiece(char[][FILES], char, int, int, int[][2]);
char *findShortRangedPiece(char[][FILES], char, int, int, int[][2]);
char *findLongRangedPiece(char[][FILES], char, int, int, int[][2]);


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
	int hasCastled[][2] = {{0, 0}, {0, 0}}; // WK, WQ, BK, BQ

	while (isPlaying) {
		display(board);
		askMove(turn, input);

		if (!strcmp(input, "quit")) {
			isPlaying = 0;
		} else if ((command = validateInput(input))) {
			if (canMove(board, turn, input, command, enPassant, hasCastled)) {
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
	printf("\n   ");
	for (int i = 0; i < FILES; i++) {
		printf(" %c", i + 'a');
	}
	printf("\n  +-----------------+\n");
	for (int i = 0; i < RANKS; i++) {
		printf("%d |", 8 - i);
		for (int j = 0; j < FILES; j++) {
			printf(" %c", board[i][j]);
		}
		printf(" | %d\n", 8 - i);
	}
	printf("  +-----------------+\n   ");
	for (int i = 0; i < FILES; i++) {
		printf(" %c", i + 'a');
	}
	printf("\n");
}

void askMove(int turn, char *reply) {
	printf("\n%s to move('quit' to quit): ", turn ? "Black" : "White");
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
char findPawnMovingTwo(char board[][FILES], int side, char *input) {
	int len = strlen(input);
	int file = getColumn(input[len-2]);

	if ((side == WHITE && input[len-1] == '4' && 
			board[5][file] == '.' && board[6][file] == 'p') ||
			(side == BLACK && input[len-1] == '5' && 
			board[2][file] == '.' && board[1][file] == 'P')) {
		return file + 'a';
	}
	return 0;
}

int canMove(char board[][FILES], int turn, char *input, int command, 
		char *enPassant, int hasCastled[][2]) {
	char *to, *from;
	int len = strlen(input);

	switch(command) {
		case 1:	from = getMovingPawn(board, turn, input);
				enPassant[turn] = findPawnMovingTwo(board, turn, input);
				break;
		case 2:	from = getCapturingPawn(board, turn, input, enPassant);
				break;
		case 3:	from = getPiece(board, turn, input, hasCastled, 0);
				break;
		case 4:	from = getPiece(board, turn, input, hasCastled, 1);
				break;
		case 5:	return canCastle(board, turn, 1, hasCastled);
		case 6:	return canCastle(board, turn, 0, hasCastled);
	}

	if (from) {
		to = getTargetSquare(board, &input[strlen(input)-2]);
		makeMove(board, to, from);
		return 1;
	}
	
	return 0;
}

int canCastle(char board[][FILES], int turn, int kingside, 
		int hasCastled[][2]) {
	int row = (turn == WHITE) ? 7 : 0;

	if (hasCastled[turn][kingside]) {
		printf("Cannot castle anymore.\n");
		return 0;
	}

	if (kingside && board[row][5] == '.' && board[row][6] == '.') {
		makeMove(board, &board[row][6], &board[row][4]);	// king
		makeMove(board, &board[row][5], &board[row][7]);	// king's rook
		hasCastled[turn][0] = hasCastled[turn][1] = 1;
		return 1;
	} else if (!kingside && board[row][1] == '.' && board[row][2] == '.' && 
			board[row][3] == '.') {
		makeMove(board, &board[row][2], &board[row][4]);	// king
		makeMove(board, &board[row][3], &board[row][0]);	// queen's rook
		hasCastled[turn][0] = hasCastled[turn][1] = 1;
		return 1;
	}

	return 0;
}

void trackCastle(char piece, int row, int col, int hasCastled[][2]) {
	int r = row == 7 ? 0 : 1;

	if ((piece == 'k' || piece == 'K') && col == 4) {
		hasCastled[r][0] = hasCastled[r][1] = 1;
	} else {
		hasCastled[r][(col == 0) ? 0 : 1] = 1;
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

int isEnemy(char c, int turn) {
	return (turn == WHITE && c >= 'A' && c < 'Z') ||
		(turn == BLACK && c >= 'a' && c < 'z');
}

char *getMovingPawn(char board[][FILES], int turn, char *input) {
	int row = getRow(input[1]);
	int col = getColumn(input[0]);

	if (board[row][col] == '.') {
		if (turn == WHITE) { 
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

char *getCapturingPawn(char board[][FILES], int turn, char *input, 
		char *enPassant) {
	int len = strlen(input);
	int row = getRow(input[len-1]);
	int col = getColumn(input[len-2]);
	char target = board[row][col];
	int file = getColumn(input[0]);

	if (file == col - 1 || file == col + 1) {
		if (target != '.' && isEnemy(target, turn)) {	// normal capture
			if (turn == WHITE && board[row+1][file] == 'p') {
				return &board[row+1][file];
			} else if (turn == BLACK && board[row-1][file] == 'P') {
				return &board[row-1][file];
			}
		} else if (enPassant[(turn+1)%2] == input[len-2]) {	// en passant
			if (turn == WHITE && isEnemy(board[row+1][col], WHITE) &&
					board[row+1][file] == 'p') {
				board[row+1][col] = '.';
				return &board[row+1][file];
			} else if (turn == BLACK && isEnemy(board[row-1][col], BLACK) &&
					board[row-1][file] == 'P') {
				board[row-1][col] = '.';
				return &board[row-1][file];
			}
		}
	}

	return 0;
}

int matchPiece(char board[][FILES], char piece, int row, int col) {
	int found = 0;
	int dim = (row == -1) ? 0 : 1;

	for (int i = 0; i < 8; i++) {
		if (piece == (dim ? board[i][col] : board[row][i])) {
			if (!found++) {
				dim = i;
			} else {
				return -1;
			}
		}
	}

	return dim >= 0 && dim <= 7;
}

char *getPiece(char board[][FILES], int turn, char *input, 
		int hasCastled[][2], int isCapturing) {
	int len = strlen(input);
	int row0 = -1;
	int col0 = -1;
	int row1 = getRow(input[len-1]);
	int col1 = getColumn(input[len-2]);
	char piece = input[0];
	char *square = 0;

	if ((!isCapturing && board[row1][col1] == '.') ||
			(isCapturing && isEnemy(board[row1][col1], turn))) {
		if (turn == WHITE) {
			piece += 32;
		}

		if ((len == 5 && !isCapturing) || (len == 6 && isCapturing)) {
			row0 = getRow(input[1]);
			col0 = getColumn(input[2]);
			if (board[row0][col0] != piece) {
				printf("Square doesn't have piece indicated.\n");
				return 0;
			}
			square = canPieceMove(board, piece, row0, col0, row1, col1);
		} else if ((len == 4 && !isCapturing) || (len == 5 && isCapturing)) {
			if	(isFile(input[1])) {
				col0 = getColumn(input[1]);
				row0 = matchPiece(board, piece, -1, col0);
			} else {
				row0 = getRow(input[1]);
				col0 = matchPiece(board, piece, row0, -1);
			}
			if (row0 == -1 || col0 == -1) {
				printf("No piece matching rank or file given.\n");
				return 0;
			}
			square = canPieceMove(board, piece, row0, col0, row1, col1);
		} else {
			return findPiece(board, piece, row1, col1, hasCastled);
		}
	}
	if (square) {
		trackCastle(piece, row0, col0, hasCastled);
	}

	return square;
}

char *canPieceMove(char board[][FILES], char piece, int row0, int col0, 
		int row1, int col1) {
	switch(piece) {
		case 'n':
		case 'N':	
		case 'k':
		case 'K':	if (canMoveShortRanged(piece, row1-row0, col1-col0)) {
						return &board[row0][col0];
					}
		case 'b':
		case 'B':
		case 'r':
		case 'R':	
		case 'q':
		case 'Q':	if (canMoveLongRanged(board, row0, col0, row1, col1)) {
						return &board[row0][col0];
					}
	}

	return 0;
}

int canMoveShortRanged(char piece, int rows, int cols) {
	for (int i = 0; i < 8; i++) {
		if (piece == 'n' || piece == 'N') {
			if (rows == knightMoves[i][0] && cols == knightMoves[i][1]) {
				return 1;
			}
		} else {
			if (rows == direction[i][0] && cols == direction[i][1]) {
				return 1;
			}
		}
	}

	printf("Move cannot be made from current position.\n");
	return 0;
}

int canMoveLongRanged(char board[][FILES], int row0, int col0, int row1, 
		int col1) {
	int r, c, i, j;
	
	r = (row1 - row0) ? ((row1 < row0) ? -1 : 1) : 0; 
	c = (col1 - col0) ? ((col1 < col0) ? -1 : 1) : 0; 

	for (i = row0 + r, j = col0 + c; i != row1 && j != col1; i += r, j += c) {
		if (board[i][j] != '.') {
			printf("Piece is being blocked.\n");
			return 0;
		}
	}

	return 1;
}

char *findPiece(char board[][FILES], char piece, int row, int col,
		int hasCastled[][2]) {
	char *square = 0;

	switch (piece) {
		case 'n':
		case 'N':
		case 'k':
		case 'K':	return findShortRangedPiece(board, piece, row, col, 
							hasCastled);
		case 'b':
		case 'B':
		case 'r':
		case 'R':	
		case 'q':
		case 'Q':	return findLongRangedPiece(board, piece, row, col, 
							hasCastled);
	}

	return 0;
}

char *findShortRangedPiece(char board[][FILES], char piece, int row1, int col1,
		int hasCastled[][2]) {
	int row0, col0, r, c;
	int found = 0;
	char *square = 0;

	for (int i = 0; i < 8; i++) {
		if (piece == 'n' || piece == 'N') {
			r = row1 + knightMoves[i][0];
			c = col1 + knightMoves[i][1];
		} else {
			r = row1 + direction[i][0];
			c = col1 + direction[i][1];
		}

		if (!isInBounds(r, c)) {
			continue;
		}
		if (board[r][c] == piece) {
			if (!found++) {
				square = &board[r][c];
				row0 = r;
				col0 = c;
			} else {
				printf("Another piece found that can make that move.\n");
				return 0;
			}
		}
	}

	if (square && (piece == 'K' || piece == 'k')) {
		trackCastle(piece, row0, col0, hasCastled);
	}

	return square;
}

char *findLongRangedPiece (char board[][FILES], char piece, int row1, int col1,
		int hasCastled[][2]) {
	int row0, col0, r, c;
	int found = 0;
	char *square = 0;
	int i = (piece == 'B' || piece == 'b') ? 1 : 0;
	int inc = (piece == 'Q' || piece == 'q') ? 1: 2;

	for (; i < 8; i += inc) {
		r = row1 + direction[i][0];
		c = col1 + direction[i][1];

		while (isInBounds(r, c) &&
				(board[r][c] == piece || board[r][c] == '.')) {
			if (board[r][c] == piece) {
				if (!found++) {
					square = &board[r][c];
					row0 = r;
					col0 = c;
				} else {
					printf("Another piece found that can make that move.\n");
					return 0;
				}
			}
			r += direction[i][0];
			c += direction[i][1];
		}
	}

	if (square && (piece == 'R' || piece == 'r')) {
		trackCastle(piece, row0, col0, hasCastled);
	}

	return square;
}
