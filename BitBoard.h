#ifndef BITBOARD_H
#define BITBOARD_H 


#include "Defs.h"

class BitBoard
{
	public:
	
	static long mask(int i);


	BitBoard(Piece p, long i): piece(p), bb(i) {};

	bool isOccupied(int i) const;

	Piece piece;
	long bb;
};



#endif 