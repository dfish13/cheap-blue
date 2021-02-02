#ifndef DEFS_H
#define DEFS_H

#define A1 0
#define B1 1
#define C1 2
#define D1 3
#define E1 4
#define F1 5
#define G1 6
#define H1 7

#define A8 56
#define B8 57
#define C8 58
#define D8 59
#define E8 60
#define F8 61
#define G8 62
#define H8 63

#include <iostream>

enum Color {none, white, black, both};
enum PType {pawn, knight, bishop, rook, queen, king, any};

struct MoveBytes
{

	MoveBytes() {};
	MoveBytes(uint8_t m, uint8_t f, uint8_t t, uint8_t d): mtype(m), from(f), to(t), detail(d) {}
	/*
		The info byte specifies what type of move it is.

		mtype & 128 = invalid
		mtype & 64 = capture
		mtype & 32 = castle
		mtype & 16 = pawn promotion
		mtype & 8 = double pawn move
		mtype & 4 = pawn move
		mtype & 2 = enpassant capture

		if the move is a castle,

		detail & 2 = castle kingside
		detail & 1 = castle queenside

		if the move is a promotion,

		detail & 8 = queen
		detail & 4 = rook
		detail & 2 = bishop
		detail & 1 = knight

	*/
	uint8_t mtype;
	uint8_t from;
	uint8_t to;
	uint8_t detail;
};

/**
 * Made to store MoveBytes. 
 * The ability to store as an int gives the ability to store as set.
 * 
 */
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