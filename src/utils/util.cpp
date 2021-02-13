#include "util.h"
#include "defs.h"
#include <iostream>

int parseSquare(std::string s)
{
	if (s.size() == 2
		&& (s[0] >= 97 && s[0] < 105)	// a-h
		&& (s[1] >= 49 && s[1] < 57)	// 1-8
	)
		return 8 * (s[1] - 49) + (s[0] - 97);
	return -1;
}


/*
	A valid move string is something of the form e2e4, b1xc3, etc.
	Does not check if the move specified is legal.

	castle kingside = O-O
	castle Queenside = O-O-O

	For promotion add =<Piece> where <Piece> is in {q, r, b, n}
	e.g. h7h8=q or e7xf8=n
*/
int parseMove(std::string m)
{
	Move move;

	uint8_t mtype = 0, from, to, detail;
	int promotion = 0;
	char promoteChars[5] = "qrbn";
	if (m == "O-O")
		move.m = {32, 0, 0, 2};
	else if (m == "O-O-O")
		move.m = {32, 0, 0, 1};
	else if (m.size() >= 4)
	{
		from = parseSquare(m.substr(0, 2));
		to = parseSquare(m.substr(2, 2));
		if (m.size() == 6)
		{
			for (int i = 0; i < 4; ++i)
				if (m[5] == promoteChars[i])
				{
					promotion = 1 << (3 - i);
					break;
				}
		}

		if (promotion){
			mtype |= 16;
		}
		detail |= promotion;
		
		if (from >= 0 && to >= 0)
			move.m = {mtype, from, to, detail};
		else 
			move.m = {128, 0, 0, 0};
	}
	else
		move.m = {128, 0, 0, 0};

	return move.x;
}