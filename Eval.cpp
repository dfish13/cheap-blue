#include "Eval.h"

namespace Eval {

/* the values of the pieces */
const int piece_value[6] = {
	100, 300, 300, 500, 900, 0
};

/* The "pcsq" arrays are piece/square tables. They're values
   added to the material value of the piece based on the
   location of the piece. */

const int pawn_pcsq[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10,   3,   2,   1,
	  0,   0,   0, -40, -40,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int knight_pcsq[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

const int bishop_pcsq[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

const int king_pcsq[64] = {
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-20, -20, -20, -20, -20, -20, -20, -20,
	  0,  20,  40, -20,   0, -20,  40,  20
};

const int king_endgame_pcsq[64] = {
	  0,  10,  20,  30,  30,  20,  10,   0,
	 10,  20,  30,  40,  40,  30,  20,  10,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 10,  20,  30,  40,  40,  30,  20,  10,
	  0,  10,  20,  30,  30,  20,  10,   0
};

/* The flip array is used to calculate the piece/square
   values for black pieces. The piece/square value of a
   white pawn is pawn_pcsq[sq] and the value of a black
   pawn is pawn_pcsq[flip[sq]] */
const int flip[64] = {
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

/* pawn_rank[x][y] is the rank of the least advanced pawn of color x on file
   y - 1. There are "buffer files" on the left and right to avoid special-case
   logic later. If there's no pawn on a rank, we pretend the pawn is
   impossibly far advanced (0 for white and 7 for black). This makes it easy to
   test for pawns on a rank and it simplifies some pawn evaluation code. */
int pawn_rank[2][10];

int piece_mat[2];  /* the value of a side's pieces */
int pawn_mat[2];  /* the value of a side's pawns */

Position pos;

int eval(Position p)
{

    // Copy p into global position to avoid passing position as an argument multiple times.
    pos = p;

    int i;
	int f;  /* file */
	int score[2];  /* each side's score */

	/* this is the first pass: set up pawn_rank, piece_mat, and pawn_mat. */
	for (i = 0; i < 10; ++i) {
		pawn_rank[white][i] = 7;
		pawn_rank[black][i] = 0;
	}
	piece_mat[white] = 0;
	piece_mat[black] = 0;
	pawn_mat[white] = 0;
	pawn_mat[black] = 0;
	for (i = 0; i < 64; ++i) {
		if (pos.squares[i].color == none)
			continue;
		if (pos.squares[i].ptype == pawn) {
			pawn_mat[pos.squares[i].color] += piece_value[pawn];
			f = COL(i) + 1;  /* add 1 because of the extra file in the array */
			if (pos.squares[i].color == white) {
				if (pawn_rank[white][f] > ROW(i))
                    pawn_rank[white][f] = ROW(i);
			}
			else {
				if (pawn_rank[black][f] < ROW(i))
					pawn_rank[black][f] = ROW(i);
			}
		}
		else
			piece_mat[pos.squares[i].color] += piece_value[pos.squares[i].ptype];
	}

	/* this is the second pass: evaluate each piece */
	score[white] = piece_mat[white] + pawn_mat[white];
	score[black] = piece_mat[black] + pawn_mat[black];
	for (i = 0; i < 64; ++i) {
		if (pos.squares[i].color == none)
			continue;
		if (pos.squares[i].color == white) {
			switch (pos.squares[i].ptype) {
				case pawn:
					score[white] += eval_white_pawn(i);
					break;
				case knight:
					score[white] += knight_pcsq[flip[i]];
					break;
				case bishop:
					score[white] += bishop_pcsq[flip[i]];
					break;
				case rook:
					if (pawn_rank[white][COL(i) + 1] == 7) {
						if (pawn_rank[black][COL(i) + 1] == 0)
							score[white] += ROOK_OPEN_FILE_BONUS;
						else
							score[white] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 6)
						score[white] += ROOK_ON_SEVENTH_BONUS;
					break;
				case king:
					if (piece_mat[black] <= 1200)
						score[white] += king_endgame_pcsq[flip[i]];
					else
						score[white] += eval_white_king(i);
					break;
			}
		}
		else {
			switch (pos.squares[i].ptype) {
				case pawn:
					score[black] += eval_black_pawn(i);
					break;
				case knight:
					score[black] += knight_pcsq[i];
					break;
				case bishop:
					score[black] += bishop_pcsq[i];
					break;
				case rook:
					if (pawn_rank[black][COL(i) + 1] == 0) {
						if (pawn_rank[white][COL(i) + 1] == 7)
							score[black] += ROOK_OPEN_FILE_BONUS;
						else
							score[black] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 1)
						score[black] += ROOK_ON_SEVENTH_BONUS;
					break;
				case king:
					if (piece_mat[white] <= 1200)
						score[black] += king_endgame_pcsq[i];
					else
						score[black] += eval_black_king(i);
					break;
			}
		}
	}

	/* the score[] array is set, now return the score relative
	   to the side to move */
	if (pos.side == white)
		return score[white] - score[black];
	return score[black] - score[white];
}

int eval_white_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

	r += pawn_pcsq[flip[sq]];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[white][f] < ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[white][f - 1] == 7) &&
			(pawn_rank[white][f + 1] == 7))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[white][f - 1] > ROW(sq)) &&
			(pawn_rank[white][f + 1] > ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[black][f - 1] <= ROW(sq)) &&
			(pawn_rank[black][f] <= ROW(sq)) &&
			(pawn_rank[black][f + 1] <= ROW(sq)))
		r += (ROW(sq)) * PASSED_PAWN_BONUS;

	return r;
}

int eval_black_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

	r += pawn_pcsq[sq];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[black][f] > ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[black][f - 1] == 0) &&
			(pawn_rank[black][f + 1] == 0))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[black][f - 1] < ROW(sq)) &&
			(pawn_rank[black][f + 1] < ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[white][f - 1] >= ROW(sq)) &&
			(pawn_rank[white][f] >= ROW(sq)) &&
			(pawn_rank[white][f + 1] >= ROW(sq)))
		r += (7 - ROW(sq)) * PASSED_PAWN_BONUS;

	return r;
}

int eval_white_king(int sq)
{
	int r;  /* the value to return */
	int i;

	r = king_pcsq[flip[sq]];

	/* if the king is castled, use a special function to evaluate the
	   pawns on the appropriate side */
	if (COL(sq) < 3) {
		r += eval_wkp(1);
		r += eval_wkp(2);
		r += eval_wkp(3) / 2;  /* problems with pawns on the c & f files
								  are not as severe */
	}
	else if (COL(sq) > 4) {
		r += eval_wkp(8);
		r += eval_wkp(7);
		r += eval_wkp(6) / 2;
	}

	/* otherwise, just assess a penalty if there are open files near
	   the king */
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[white][i] == 7) &&
					(pawn_rank[black][i] == 0))
				r -= 10;
	}

	/* scale the king safety value according to the opponent's material;
	   the premise is that your king safety can only be bad if the
	   opponent has enough pieces to attack you */
	r *= piece_mat[black];
	r /= 3100;

	return r;
}

/* eval_lkp(f) evaluates the white King Pawn on file f */

int eval_wkp(int f)
{
	int r = 0;

	if (pawn_rank[white][f] == 1);  /* pawn hasn't moved */
	else if (pawn_rank[white][f] == 2)
		r -= 10;  /* pawn moved one square */
	else if (pawn_rank[white][f] != 7)
		r -= 20;  /* pawn moved more than one square */
	else
		r -= 25;  /* no pawn on this file */

	if (pawn_rank[black][f] == 0)
		r -= 15;  /* no enemy pawn */
	else if (pawn_rank[black][f] == 2)
		r -= 10;  /* enemy pawn on the 3rd rank */
	else if (pawn_rank[black][f] == 3)
		r -= 5;   /* enemy pawn on the 4th rank */

	return r;
}

int eval_black_king(int sq)
{
	int r;
	int i;

	r = king_pcsq[sq];
	if (COL(sq) < 3) {
		r += eval_bkp(1);
		r += eval_bkp(2);
		r += eval_bkp(3) / 2;
	}
	else if (COL(sq) > 4) {
		r += eval_bkp(8);
		r += eval_bkp(7);
		r += eval_bkp(6) / 2;
	}
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[white][i] == 7) &&
					(pawn_rank[black][i] == 0))
				r -= 10;
	}
	r *= piece_mat[white];
	r /= 3100;
	return r;
}

int eval_bkp(int f)
{
	int r = 0;

	if (pawn_rank[black][f] == 6);
	else if (pawn_rank[black][f] == 5)
		r -= 10;
	else if (pawn_rank[black][f] != 0)
		r -= 20;
	else
		r -= 25;

	if (pawn_rank[white][f] == 7)
		r -= 15;
	else if (pawn_rank[white][f] == 5)
		r -= 10;
	else if (pawn_rank[white][f] == 4)
		r -= 5;

	return r;
}

} // namespace Eval

