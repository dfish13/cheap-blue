#include <iostream>

#include <utility>
#include <cstdint>
#include "Board.h"




using namespace std;






int parseSquare(string s)
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
int parseMove(string m)
{
	Move move;

	int info = 0, from, to;
	int capture = 0, promotion = 0;
	char promoteChars[5] = "qrbn";
	if (m == "O-O")
		move.m = {34, 0, 0};
	else if (m == "O-O-O")
		move.m = {33, 0, 0};
	else if (m.size() >= 4)
	{
		from = parseSquare(m.substr(0, 2));
		if (m[2] == 'x')
			capture = 1;
		to = parseSquare(m.substr(2 + capture, 2));
		if (m.size() == 6 + capture)
		{
			for (int i = 0; i < 4; ++i)
				if (m[5 + capture] = promoteChars[i])
				{
					promotion = 1 << (3 - i);
					break;
				}
		}
		
		if (capture)
			info |= 64;
		if (promotion)
			info |= 16;
		info |= promotion;
		if (from >= 0 && to >= 0)
			move.m = {info, from, to};
		else
			move.m = {128, 0, 0};
	}
	else
		move.m = {128, 0, 0};
	return move.x;
}



int main()
{

	Move move;
	string s;
	Board board;
	board.init();

	while (1)
	{
		board.display(cout);
		cout << "Enter move or q to quit: ";
		while (1)
		{
			cin >> s;
			move.x = parseMove(s);
			if (s == "q")
			{
				cout << "Bye!\n";
				exit(0);
			}
			else if (move.m.info & 128)
			{
				cout << "Invalid move string. Try again: ";
				continue;
			}	
			else if (!board.makeMove(move))
			{
				cout << "Illegal move. Try again: ";
				continue;
			}
			break;	
		}
	}
	
	return 0;
}
