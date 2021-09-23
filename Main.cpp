#include <iostream>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "Game.h"
#include "Util.h"
#include "Test.h"
#include "Engine.h"

using namespace std;

void test();
void perft(Game & game, int depth);
void menu();

int main(int argc, char ** argv)
{

	string fen;
	Game game;
	Move move;
	int depth;

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
		else if (arg == "-m" && argc > 2)
		{
			// For now, just pick a random move from the set of possible moves.
			fen = argv[2];
			game.init(fen);
			Engine engine(&game);
			engine.think(3000);
			move = engine.move();
			cout << getMoveString(move);
			return 0;
		}
		else if (arg == "-p" && argc > 3)
		{
			depth = stoi(argv[2]);
			fen = argv[3];
			game.init(fen);
			if (depth > 0)
				perft(game, depth);
			else
				cout << "Depth must be > 0\n";
			return 0;
		}
		else
		{
			cout << "Unrecognized command line argument\n";
			cout << "Valid options are:\n\n";
			cout << "\t-t: run tests\n";
			cout << "\t-m \"<fen>\": get engine move\n";
			cout << "\t-p <depth> \"<fen>\": run perft test\n";
			cout << "\t-f \"<fen>\": load game from FEN string\n\n";
			return 0;
		}
	}

	string s;
	if (fen.empty())
		game.init();
	else
		game.init(fen);

	Engine engine(&game, &cout);

	cout << "Type m to print menu\n";
	while (1)
	{
		cout << "> ";
		cin >> s;
		if (s == "q")
		{
			cout << "Bye!\n";
			exit(0);
		}
		else if (s == "d")
		{
			game.display(cout);
		}
		else if (s == "m")
		{
			menu();
		}
		else if (s == "b")
		{
			if (!game.takeBack())
			{
				cout << "Unable to take back move. Try something else.\n";
				continue;
			}
		}
		else if (s == "p")
		{
			cout << "depth = ";
			cin >> depth;
			while (depth < 1)
			{
				cout << "Depth must be > 1. Try again: ";
				cin >> depth;
			}
			perft(game, depth);
		}
		else if (s == "i")
		{
			printPosition(cout, game.pos);
		}
		else if (s == "e")
		{
			cout << "static evaluation = " << game.Evaluation() << '\n';
		}
		else if (s == "c")
		{
			engine.think(5000);
			move = engine.move();
			cout << getMoveString(move) << '\n';
		}
		else
		{
			move.x = parseMove(s);
			if (move.m.mtype & 128)
			{
				cout << "Invalid command or move string. Try again.\n";
				continue;
			}
			move = game.getMove(move.x);
			if (!game.makeMove(move))
				cout << "Illegal move. Try again.\n";
		}
	}

	return 0;
}

void test()
{
	testEval();
	testPerft();
	testBoardIsAttacked();
	testIntegration();
	testFENParser();
	testInCheck();
}

void perft(Game & game, int depth)
{
	Move move;
	cout << "Perft " << depth << '\n';
	vector<int> moves;
	moves.reserve(40);
	game.genMoves(moves);
	long nodes, totalNodes = 0;
	for (int x: moves)
	{
		move.x = x;
		if (game.makeMove(move))
		{
			nodes = game.Perft(depth - 1);
			cout << getMoveString(move) << ' ' << nodes << '\n';
			totalNodes += nodes;
			game.takeBack();
		}
	}
	cout << "Total nodes = " << totalNodes << '\n';
}

void menu()
{
	cout << "\n\tEnter a move (e.g. e2e4)\n";
	cout << "\tm to print menu\n";
	cout << "\tb to take a move back\n";
	cout << "\tp to run perft test\n";
	cout << "\ti to print info on current position\n";
	cout << "\te to print static evaluation\n";
	cout << "\td to display board\n";
	cout << "\tc to get computer move\n";
	cout << "\tq to quit\n";
}
