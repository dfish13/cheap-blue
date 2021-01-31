#ifndef BOARD_H
#define BOARD_H 


#include <vector>
#include <set>
#include <iostream>
#include <utility>
#include <cstdint>

#include "BitBoard.h"
#include "Defs.h"


class Board
{
	public:

	Board() {};

	void init();


	bool checkMove(Move m) const;

	bool makeMove(Move m);

	set<int> genMoves() const;

	// Returns -1 if not a valid piece
	static int getPieceIndex(Piece p);

	static char getPieceLetter(Piece p);

	/*
		Same display board function as TSCP.
		TSCP was the first chess engine I examined so as I develop my own
		engine, I'm sure I will get a lot of help and inspiration from TSCP.

		http://www.tckerrigan.com/Chess/TSCP/
	*/
	void display(ostream & os) const;

    const int mailbox[120] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	-1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	-1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	-1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	-1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	-1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	-1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };

    const int mailbox64[64] = {
    91, 92, 93, 94, 95, 96, 97, 98,
	81, 82, 83, 84, 85, 86, 87, 88,
	71, 72, 73, 74, 75, 76, 77, 78,
	61, 62, 63, 64, 65, 66, 67, 68,
	51, 52, 53, 54, 55, 56, 57, 58,
	41, 42, 43, 44, 45, 46, 47, 48,
	31, 32, 33, 34, 35, 36, 37, 38,
	21, 22, 23, 24, 25, 26, 27, 28
    };

	vector<BitBoard> bitBoards;
	vector<Piece> squares;

    //side has the move, xside is other.
	Color side, xside;
    //half move counter.
	int hply;
    //The value of the square that can be taken with enpassant.
	int enpassant;
    /*
        This is for storing the right to castle, not necessarily the possibility in the board state.
        ex. ) Pieces still in between may result in true, but castling cannot be done.

	    castle & 8 != 0 => white can long castle(O-O-O)
	    castle & 4 != 0 => white can short castle(O-O)
	    castle & 2 != 0 => black can long castle(O-O-O)
	    castle & 1 != 0 => black can short castle(O-O)
    */
	uint8_t castleRights;
};



#endif 