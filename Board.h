#ifndef BOARD_H
#define BOARD_H 


#include <vector>
#include <set>
#include <iostream>
#include <utility>
#include <cstdint>
#include <cstdlib>

#include "BitBoard.h"
#include "Defs.h"


class Board
{
public:

	Board() {};

	void init();
	void init(std::string fen);

	bool makeMove(Move m);

	bool takeBack();

	// Returns true if square is attacked by a piece of color c.
	bool isAttacked(int square, Color c) const;

	/*
		Because the notation for a user to input a move contains incomplete information,
		it is necessary to match it up to a move created in the genMoves function which contains complete
		information about the move type.

		At the moment this function is coupled with the parseMove function in Util.h which is undesirable,
		but I don't see a way around it without changing the move input notation.
	*/
	Move getMove(int m) const;

	std::set<int> genMoves() const;

	/*
		Same display board function as TSCP.
		TSCP was the first chess engine I examined so as I develop my own
		engine, I'm sure I will get a lot of help and inspiration from TSCP.

		http://www.tckerrigan.com/Chess/TSCP/
	*/
	void display(std::ostream & os) const;

	Move getRandomMove() const;

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

	/*
	Used to generate moves.

	Each array has 6 entries for the 6 possible pieces:
	Pawn, Knight, Bishop, Rook, Queen, King

	The slide array tells you whether a piece can move continuously in one direction.
	i.e. the Queen can "slide" and the king cannot because it only moves one space.

	offsets tells you how many directions it is possible for a piece to move.
	offset tells you the directions a piece can move within the mailbox representation.
	*/
	const bool slide[6] = {false, false, true, true, true, false};
	const int offsets[6] = {0, 8, 4, 4, 8, 8};
	const int offset[6][8] = {
		{   0,   0,  0,  0, 0,  0,  0,  0 },
		{ -21, -19,-12, -8, 8, 12, 19, 21 },
		{ -11,  -9,  9, 11, 0,  0,  0,  0 },
		{ -10,  -1,  1, 10, 0,  0,  0,  0 },
		{ -11, -10, -9, -1, 1,  9, 10, 11 },
		{ -11, -10, -9, -1, 1,  9, 10, 11 }
	};

	Position pos;

private:

	static std::set<int> generatePawnPromotionMoves(uint8_t from, uint8_t to);

};



#endif 