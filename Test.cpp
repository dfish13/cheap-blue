#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "Defs.h"
#include "Util.h"
#include "Board.h"

using namespace std;

int main()
{  

    string s, tok, description;
    ifstream fin;
    fin.open("tests");

    int testCounter = 0;

    Move move;
	Board board;
    bool pass;
	

    while (getline(fin, s))
    {
        pass = true;
        board.init();
        ++testCounter;
        istringstream iss(s);
        
        iss >> description;
        while (iss >> tok)
        {
            move.x = parseMove(tok);
            if (!board.makeMove(move))
			{
				pass = false;
				break;
			}
        }

        if (pass)
            cout << "Test " << testCounter << " passed (" << description << ")\n";
        else
            cout << "Test " << testCounter << " failed (" << description << ")\n";
        
    }

    return 0;
}