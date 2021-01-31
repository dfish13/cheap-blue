#include <iostream>
#include <utility>
#include <cstdint>

#include "Board.h"
#include "Util.h"


using namespace std;


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
