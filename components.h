#ifndef COMPONENTS_H
#define COMPONENTS_H 

#include <iostream>

enum Color {none, white, black, both};
enum PType {pawn, knight, bishop, rook, queen, king, any};

struct MoveBytes
{

	MoveBytes() {};
	MoveBytes(uint8_t i, uint8_t f, uint8_t t): info(i), from(f), to(t), pad(0) {}
	/*
		The info byte specifies what type of move it is.

		info & 128 = invalid
		info & 64 = capture
		info & 32 = castle
		info & 16 = promotion

		if the move is a castle,

		info & 2 = castle kingside
		info & 1 = castle queenside

		if the move is a promotion,

		info & 8 = queen
		info & 4 = rook
		info & 2 = bishop
		info & 1 = knight

	*/
	uint8_t info;
	uint8_t from;
	uint8_t to;
	uint8_t pad;
};

union Move
{
	Move() {};
	MoveBytes m;
	int x;
};


struct Piece
{
	Piece()
	{
		color = none;
		ptype = any;
	}

	Piece(Color c, PType p): color(c), ptype(p) {}
	Color color;
	PType ptype;
};



#endif 