#include <iostream>
#include <utility>
#include <cstdint>

#include "board.h"
#include "utils/util.h"
#include "test/test.h"


using namespace std;


int main(int argc, char *argv[])
{
	// run tests.
	if(argc > 1  && *argv[1] == 't')
	{
		cout << argv[1] << endl;
		return Test::mainTest();
	}

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
