#include <stdio.h>
#include <string.h>	// strlen, strcmp

#define RANKS 8
#define FILES 8
#define MAX_CHAR 8
#define MAX_PIECES 16

const char *pieces = "KQRBN";
const int direction[][8][2] = {
	{{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}},
	{{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}}
};

enum player{ WHITE, BLACK };

void display(char [][FILES]);
void askMove(int, char *);
int validateInput(char *);
int validatePawnMove(char *, int);
int validatePieceMove(char *, int);
int validateCastling(char *);
int isFile(char);
int isRank(char);
int isAlgebraic(char);
int isSquare(char *);
int isInBounds(int, int);
int canMove(char[][FILES], int, char *, int, char *, int[][2], int *, 
	int[][2], char *);
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
char *getMovingPawn(char[][FILES], int, char *, char *);
char *getCapturingPawn(char[][FILES], int, char *, char *, char *);
char promotePawn(char *, int);
int matchPiece(char[][FILES], char, int, int);
char *getPiece(char[][FILES], int, char *, int[][2], int);
char *findPiece(char[][FILES], char, int, int, int[][2]);
int isCheck(char[][FILES], int, int, int, int);
int isMate(char board[][FILES], int, int, int, int, int);
void copyBoard(char[][FILES], char[][FILES]);


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
	int checked = 0;
	int result;
	int kings[][2] = {{7, 4}, {0, 4}};
	char promotion;

	while (isPlaying) {
		display(board);
		askMove(turn, input);

		if (!strcmp(input, "quit")) {
			isPlaying = 0;
		} else if ((command = validateInput(input))) {
			if ((result = canMove(board, turn, input, command, enPassant, 
					hasCastled, &checked, kings, &promotion))) {
				printf("%d.%s%s", moves, turn ? ".. " : " ", input);
				if (promotion) {
					printf("=%c", promotion);
				}
				printf("%s\n", (!checked) ? "" : (checked < 0 ? "#" : "+"));
				if (checked == -1) {
					printf("\nCheckmate. %s\n", turn ? "0-1" : "1-0");
					display(board);
					break;
				}
				enPassant[turn^1] = 0;
			} else {
				printf("Illegal move.\n");
				continue;
			}
		} else {
			printf("Invalid input.\n");
			continue;
		}

		if (turn ) {
			moves++;
		}
		turn ^= 1;
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
	printf("\n\n");
}

void askMove(int turn, char *reply) {
	printf("%s to move('quit' to quit): ", turn ? "Black" : "White");
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
		return validateCastling(str);
	} else if (isSquare(&str[len-2])) {
		if (isFile(c)) {	// "abcdefgh" - pawn move
			return validatePawnMove(str, len);
		} else {	// "NBRQK" - piece move
			i = 0;
			while (pieces[i]) {
				if (c == pieces[i++]) {
					return validatePieceMove(str, len);
				}
			}
		}
	}

	return 0;
}

int validatePawnMove(char *str, int len) {
	if (len == 2) {
		return 1;	// pawn move
	} else if (len == 4 && str[1] == 'x') {
		return 2;	// pawn capture
	}

	return 0;
}

int validatePieceMove(char *str, int len) {
	if (len == 3 || (len == 4 && isAlgebraic(str[1])) ||
			(len == 5 && isRank(str[1]) && isFile(str[2]))) {
		return 3;	// piece move
	} else if ((str[1] == 'x' && len == 4) ||
			(str[2] == 'x' && len == 5 && isAlgebraic(str[1])) ||
			(str[3] == 'x' && len == 6 && isRank(str[1]) && isFile(str[2]))) {
		return 4;	// piece capture
	}

	return 0;
}

int validateCastling(char *str) {
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

int canMove(char board[][FILES], int turn, char *input, int command, 
		char *enPassant, int hasCastled[][2], int *checked, int kings[][2],
		char *promotion) {
	char *to, *from;
	int len = strlen(input);
	char temp;
	char *captured = 0;
	*promotion = 0;

	switch(command) {
		case 1:	from = getMovingPawn(board, turn, input, &enPassant[turn]);
				break;
		case 2:	from = getCapturingPawn(board, turn, input, enPassant, 
					captured);
				break;
		case 3:	from = getPiece(board, turn, input, hasCastled, 0);
				break;
		case 4:	from = getPiece(board, turn, input, hasCastled, 1);
				break;
		case 5:	return canCastle(board, turn, 1, hasCastled);
		case 6:	return canCastle(board, turn, 0, hasCastled);
	}

	if (from) {
		if (input[0] == 'k' || input[0] == 'K') {
			kings[turn][0] = getRow(input[strlen(input)-1]);
			kings[turn][1] = getColumn(input[strlen(input)-2]);
		}
		to = getTargetSquare(board, &input[strlen(input)-2]);
		temp = *to;
		makeMove(board, to, from);
		if (captured) {
			*captured = '.';
		}
		if (command == 1 || command == 2) {
			if ((turn == WHITE && input[len-1] == '8') ||
					(turn == BLACK && input[len-1] == '1')) {
				*promotion = promotePawn(to, turn);
			}
		}
		if (isCheck(board, turn, kings[turn][0], kings[turn][1], 0)) {
			printf("Move puts own king in check.\n");
			makeMove(board, from, to);
			*to = temp;
			*captured = turn ? 'P' : 'p';
			return 0;
		}
		*checked = isCheck(board, turn^1, kings[turn^1][0], kings[turn^1][1],
			1);

		return 1;
	}
	
	return 0;
}

int canCastle(char board[][FILES], int turn, int kingside, 
		int hasCastled[][2]) {
	int row = (turn == WHITE) ? 7 : 0;

	if (hasCastled[turn][kingside]) {
		printf("Castling is no longer allowed.\n");
		return 0;
	}

	if (kingside && board[row][5] == '.' && 
			!isCheck(board, turn, row, 5, 0) &&
			board[row][6] == '.' && !isCheck(board, turn, row, 6, 0)) {
		makeMove(board, &board[row][6], &board[row][4]);	// king
		makeMove(board, &board[row][5], &board[row][7]);	// king's rook
		hasCastled[turn][0] = hasCastled[turn][1] = 1;
		return 1;
	} else if (!kingside && board[row][1] == '.' && 
			board[row][2] == '.' && !isCheck(board, turn, row, 2, 0) &&
			board[row][3] == '.' && !isCheck(board, turn, row, 3, 0)) {
		makeMove(board, &board[row][2], &board[row][4]);	// king
		makeMove(board, &board[row][3], &board[row][0]);	// queen's rook
		hasCastled[turn][0] = hasCastled[turn][1] = 1;
		return 1;
	}
	printf("It is not clear to castle.\n");

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

char *getMovingPawn(char board[][FILES], int turn, char *input, 
		char *enPassant) {
	int row = getRow(input[1]);
	int col = getColumn(input[0]);
	*enPassant = 0;

	if (board[row][col] == '.') {
		if (turn == WHITE) { 
			if (board[row+1][col] == 'p') {
			   return &board[row+1][col];
			} else if (row == 4 && board[row+1][col] == '.' && 
					board[6][col] == 'p') {
				*enPassant = input[0];
			   return &board[6][col];
			}
		} else {
			if (board[row-1][col] == 'P') {
				return &board[row-1][col];
			} else if (row == 3 && board[row-1][col] == '.' && 
					board[1][col] == 'P') {
				*enPassant = input[0];
				return &board[1][col];
			}
		}
	}

	return 0;
}

char *getCapturingPawn(char board[][FILES], int turn, char *input, 
		char *enPassant, char *captured) {
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
		} else if (enPassant[turn^1] == input[len-2]) {	// en passant
			if (turn == WHITE && isEnemy(board[row+1][col], WHITE) &&
					board[row+1][file] == 'p') {
				captured = &board[row+1][col];
				return &board[row+1][file];
			} else if (turn == BLACK && isEnemy(board[row-1][col], BLACK) &&
					board[row-1][file] == 'P') {
				captured = &board[row-1][col];
				return &board[row-1][file];
			}
		}
	}

	return 0;
}

char promotePawn(char *to, int turn) {
	char c;

	do {
		printf("Promote pawn('QRBN'): ");
		getchar();
		c = getchar();
		if (c >= 'a' && c <= 'z') {
			c -= 32;
		}
	} while (c != 'Q' && c != 'R' && c != 'B' && c != 'N');

	*to = (turn == WHITE) ? (c + 32) : c;

	return c;
}

int matchPiece(char board[][FILES], char piece, int row, int col) {
	int found = 0;
	int dim = (row == -1) ? 0 : 1;
	int missing = -1;

	for (int i = 0; i < 8; i++) {
		if (piece == (dim ? board[row][i] : board[i][col])) {
			if (!found++) {
				missing = i;
			} else {
				return -1;
			}
		}
	}

	return missing;
}

char *getPiece(char board[][FILES], int turn, char *input, 
		int hasCastled[][2], int isCapturing) {
	int len = strlen(input);
	int row0 = -1;
	int col0 = -1;
	int row1 = getRow(input[len-1]);
	int col1 = getColumn(input[len-2]);
	char piece = (turn == WHITE) ? (input[0] + 32) : input[0];
	char *square = 0;

	if ((!isCapturing && board[row1][col1] == '.') ||
			(isCapturing && isEnemy(board[row1][col1], turn))) {

		if ((len == 5 && !isCapturing) || (len == 6 && isCapturing)) {
			row0 = getRow(input[1]);
			col0 = getColumn(input[2]);
			if (board[row0][col0] != piece) {
				printf("Piece is not located in that square.\n");
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
				printf("There is no piece matching rank or file given.\n");
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
	if ((piece == 'n' || piece == 'N' || piece == 'k' || piece == 'K') &&
		(canMoveShortRanged(piece, row1-row0, col1-col0))) {
			return &board[row0][col0];
	} else if (canMoveLongRanged(board, row0, col0, row1, col1)) {
		return &board[row0][col0];
	}
	printf("The piece cannot move to this square.\n");

	return 0;
}

int canMoveShortRanged(char piece, int rows, int cols) {
	int i = (piece == 'n' || piece == 'N') ? 1 : 0;

	for (int j = 0; j < 8; j++) {
		if (rows == direction[i][j][0] && cols == direction[i][j][1]) {
			return 1;
		}
	}

	return 0;
}

int canMoveLongRanged(char board[][FILES], int row0, int col0, int row1, 
		int col1) {
	int r, c, i, j;
	
	r = (row1 - row0) ? ((row1 < row0) ? -1 : 1) : 0; 
	c = (col1 - col0) ? ((col1 < col0) ? -1 : 1) : 0; 

	for (i = row0 + r, j = col0 + c; i != row1 && j != col1; i += r, j += c) {
		if (board[i][j] != '.') {
			return 0;
		}
	}

	return 1;
}

char *findPiece (char board[][FILES], char piece, int row1, int col1,
		int hasCastled[][2]) {
	int row0, col0, r, c;
	int found = 0;
	char *square = 0;
	int i = (piece == 'N' || piece == 'n') ? 1 : 0;
	int j = (piece == 'B' || piece == 'b') ? 1 : 0;
	int inc = (piece == 'B' || piece == 'b' || piece == 'R' || piece == 'r') ? 
		2 : 1;

	for (; j < 8; j += inc) {
		r = row1 + direction[i][j][0];
		c = col1 + direction[i][j][1];

		while (isInBounds(r, c) &&
				(board[r][c] == piece || board[r][c] == '.')) {
			if (board[r][c] == piece) {
				if (!found++) {
					square = &board[(row0=r)][(col0=c)];
				} else {
					printf("Another piece found that can make same move.\n");
					return 0;
				}
			}
			if (piece == 'N' || piece == 'n' || piece == 'K' || piece == 'k') {
				break;
			}
			r += direction[i][j][0];
			c += direction[i][j][1];
		}
	}

	if (square && (piece == 'R' || piece == 'r' || piece == 'K' || 
			piece == 'k')) {
		trackCastle(piece, row0, col0, hasCastled);
	}

	return square;
}

int isCheck(char board[][FILES], int turn, int row, int col, int mateCheck) {
	int r, c, atkRow, atkCol, square;
	int checked = 0;
	char hypothetical[RANKS][FILES];

	// pawns
	if (turn == WHITE && row - 1 > 0 &&
			((col - 1 >= 0 && board[(atkRow=row-1)][(atkCol=col-1)] == 'P') ||
			(col + 1 <= 7 && board[(atkRow=row-1)][(atkCol=col+1)] == 'P'))) {
		if (!mateCheck) {
			return (atkRow * RANKS) + atkCol + 1;
		} else if (isMate(board, turn, row, col, atkRow, atkCol)) {
			return -1;
		}
		checked++;
	} else if (turn == BLACK && row + 1 < 7 &&
			((col - 1 >= 0 && board[(atkRow=row+1)][(atkCol=col-1)] == 'p') ||
			(col + 1 <= 7 && board[(atkRow=row+1)][(atkCol=col+1)] == 'p'))) {
		if (!mateCheck) {
			return (atkRow * RANKS) + atkCol + 1;
		} else if (isMate(board, turn, row, col, atkRow, atkCol)) {
			return -1;
		}
		checked++;
	}
	// pieces 
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 8; j++) {
			r = row + direction[i][j][0];
			c = col + direction[i][j][1];
			if (!isInBounds(r, c)) {
				continue;
			}
			if ((i == 0 && ((turn == WHITE && board[r][c] == 'K') ||
					(turn == BLACK && board[r][c] == 'k'))) ||
					(i == 1 && ((turn == WHITE && board[r][c] == 'N') ||
					(turn == BLACK && board[r][c] == 'n')))) {
				if (!mateCheck) {
					if ((board[r][c] == 'K' && turn == WHITE) ||
							(board[r][c] == 'k' && turn == BLACK)) {
						copyBoard(hypothetical, board);
						makeMove(hypothetical, &hypothetical[row][col], 
								&hypothetical[r][c]);
						if (isCheck(hypothetical, turn^1, row, col, 0)) {
							continue;
						}
					}
					return (r * RANKS) + c + 1;
				} else if (isMate(board, turn, row, col, r, c)) {
					return -1;
				}
				checked++;
			}
			while (isInBounds(r, c)) {
				if (board[r][c] != '.') {
					if (i == 0 &&
							((turn == WHITE && (board[r][c] == 'Q' || 
							(j % 2 == 0 && board[r][c] == 'R') ||
							(j % 2 == 1 && board[r][c] == 'B'))) ||
							(turn == BLACK && (board[r][c] == 'q'||
							(j % 2 == 0 && board[r][c] == 'r') ||
							(j % 2 == 1 && board[r][c] == 'b'))))) {
						if (!mateCheck) {
							return (r * RANKS) + c + 1;
						} else if (isMate(board, turn, row, col, r, c)) {
							return -1;
						}
						checked++;
					} else {
						break;
					}
				}
				r += direction[i][j][0];
				c += direction[i][j][1];
			}
		}
	}
	return checked;
}

int isMate(char board[][FILES], int turn, int kRow, int kCol, 
		int atkRow, int atkCol) {
	int i, j, r, c, square, difRow, difCol;
	char hypothetical[RANKS][FILES];

	// check for square that king can escape to
	for (i = 0; i < 8; i++) {
		r = direction[0][i][0] + kRow;
		c = direction[0][i][1] + kCol;

		if ((board[r][c] == '.' || isEnemy(board[r][c], turn)) &&
				!isCheck(board, turn, r, c, 0)) {
			return 0;	// can escape
		}
	}

	// check if attacking piece can be captured
	// set up hypothetical board to check if capture can be made safely
	copyBoard(hypothetical, board);

	if ((square = isCheck(board, turn^1, atkRow, atkCol, 0))) {
		makeMove(hypothetical, &hypothetical[atkRow][atkCol], 
			&hypothetical[(square-1)/8][(square-1)%8]);
		// attacking piece can be captured
		if (!isCheck(hypothetical, turn, kRow, kCol, 0)) {
			return 0;
		}
	}

	// check if attacking piece (bishops, rooks, queen) can be blocked instead
	if (board[atkRow][atkCol] == 'B' || board[atkRow][atkCol] == 'b' ||
			board[atkRow][atkCol] == 'R' || board[atkRow][atkCol] == 'r' ||
			board[atkRow][atkCol] == 'Q' || board[atkRow][atkCol] == 'q') {
		copyBoard(hypothetical, board);
		difRow = atkRow - kRow;
		difCol = atkCol - kCol;

		//check path between attacker and king
		i = (difRow ? (difRow < 0 ? -1 : 1) : 0);
		j = (difCol ? (difCol < 0 ? -1 : 1) : 0);
		r = kRow + i;
		c = kCol + j; 
		while (!(r == atkRow && c == atkCol)) {
			square = isCheck(hypothetical, turn, r, c, 0);
			makeMove(hypothetical, &hypothetical[atkRow][atkCol], 
				&hypothetical[--square/8][square%8]);
				if (!isCheck(hypothetical, turn, kRow, kCol, 0)) {
					return 0;
				}
			r += i;
			c += j;
		}
	}

	return 1;
}

void copyBoard(char to[][FILES], char from[][FILES]) {
	for (int i = 0; i < RANKS; i++) {
		for (int j = 0; j < FILES; j++) {
			to[i][j] = from[i][j];
		}
	}
}
