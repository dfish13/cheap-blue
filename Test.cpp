#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "Defs.h"
#include "Util.h"
#include "Board.h"

using namespace std;

void testBoardIsAttacked()
{
    Move m;
    Board board;
    board.init();


    cout << board.isAttacked(20, white) << endl;
    cout << board.isAttacked(21, white) << endl;
    m.x = parseMove("e2e4");
    board.makeMove(m);

    cout << board.isAttacked(35, white) << endl;
    cout << board.isAttacked(39, white) << endl;
    cout << board.isAttacked(40, white) << endl;
    m.x = parseMove("b8c6");
    board.makeMove(m);

    cout << board.isAttacked(27, black) << endl;
    cout << board.isAttacked(28, black) << endl;

}

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

    testBoardIsAttacked();

    return 0;
}