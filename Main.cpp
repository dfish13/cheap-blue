#include <iostream>
#include <utility>
#include <cstdint>

#include "Board.h"
#include "Util.h"
#include "Test.h"


using namespace std;

void test();

int main(int argc, char ** argv)
{

	string fen;
	// If there are any command line arguments
	if (argc > 1)
	{
		string arg(argv[1]);
		if (arg == "-t")
		{
			test();
			return 0;
		}
		else if (arg == "-f" && argc > 2)
		{
			fen = argv[2];
		}
		else
		{
			cout << "Unrecognized command line argument\n";
			cout << "Valid options are:\n\n";
			cout << "\t-t: run tests\n";
			cout << "\t-f \"<fen>\": load FEN string\n\n";
			return 0;
		}
	}

	Move move;
	string s;
	Board board;
	if (fen.empty())
		board.init();
	else
		board.init(fen);

	while (1)
	{
		board.display(cout);
		cout << "Enter move or q to quit: ";
		while (1)
		{
			cin >> s;
			move = board.getMove(parseMove(s));
			if (s == "q")
			{
				cout << "Bye!\n";
				exit(0);
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

void test()
{
	testBoardIsAttacked();
	testIntegration();
}
