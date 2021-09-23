/**
 * Contains definitions for common structures and values to be used
 * throughout Cheap Blue.
 * 
 */

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

#define NSQUARES 64

#define ROW(x)	(x >> 3)
#define COL(x)	(x & 7)

#include <cstdint>

enum Color {white, black, none, both};
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

// Unioned with int so that you can easily convert between the two types. 
union Move
{
	Move() {};
	Move(int x) : x(x) {};
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

	bool operator==(Piece p)
	{
		return color == p.color && ptype == p.ptype;
	}

	Piece(Color c, PType p): color(c), ptype(p) {}
	Color color;
	PType ptype;
};


// Needs to be Initialized with either defaultPosition or getPositionFromFEN
// in Util.h
struct Position
{
	Position()
	{

	}

	Piece squares[NSQUARES];
	// Side has the move, xside is other.
	Color side, xside;
    // The value of the square that can be taken with enpassant.
	uint8_t enpassant;
    /*
        This is for storing the right to castle, not necessarily the possibility in the board state.
        ex. ) Pieces still in between may result in true, but castling cannot be done.

	    castle & 8 != 0 => white can long castle(O-O-O)
	    castle & 4 != 0 => white can short castle(O-O)
	    castle & 2 != 0 => black can long castle(O-O-O)
	    castle & 1 != 0 => black can short castle(O-O)
    */
	uint8_t castleRights;

	// Number of half moves made since the last irreversible move. 50 (100 half moves) in a row means the game is a draw.
	int fifty;

	// Zobrist hash of the position
	int hash;

};

// Contains information that allows you to take a move back.
struct MoveInfo
{
	Move m;
	PType capture;
	uint8_t castleRights;
	int ep;
	int fifty;
	int hash;
};



#endif 