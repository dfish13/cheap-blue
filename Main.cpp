#include <iostream>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "Game.h"
#include "Util.h"
#include "Test.h"


using namespace std;

void test();

int main(int argc, char ** argv)
{

	string fen;
	Game game;
	Move move;
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
			// For now, just pick a random move from the set of possible moves.
			fen = argv[2];
			game.init(fen);
			move = game.getRandomMove();
			cout << getMoveString(move);
			return 0;
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

	string s;
	game.init();

	while (1)
	{
		game.display(cout);
		cout << "Enter move, b to take back move, or q to quit: ";
		while (1)
		{
			cin >> s;
			move = game.getMove(parseMove(s));
			if (s == "q")
			{
				cout << "Bye!\n";
				exit(0);
			}
			else if (s == "b")
			{
				if (!game.takeBack())
				{
					cout << "Unable to take back move. Try something else: ";
					continue;
				}
			}
			else if (!game.makeMove(move))
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
	testFENParser();
	testInCheck();
}
