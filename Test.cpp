#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "Defs.h"
#include "Util.h"
#include "Board.h"
#include "Test.h"

void testBoardIsAttacked()
{
    Move m;
    Board board;
    board.init();


    std::cout << board.isAttacked(20, white) << std::endl;
    std::cout << board.isAttacked(21, white) << std::endl;
    m.x = parseMove("e2e4");
    board.makeMove(m);

    std::cout << board.isAttacked(35, white) << std::endl;
    std::cout << board.isAttacked(39, white) << std::endl;
    std::cout << board.isAttacked(40, white) << std::endl;
    m.x = parseMove("b8c6");
    board.makeMove(m);

    std::cout << board.isAttacked(27, black) << std::endl;
    std::cout << board.isAttacked(28, black) << std::endl;

}

void testIntegration()
{  

    std::string s, tok, description;
    std::ifstream fin;
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
        std::istringstream iss(s);
        
        iss >> description;
        while (iss >> tok)
        {
            move = board.getMove(parseMove(tok));
            if (!board.makeMove(move))
			{
				pass = false;
				break;
			}
        }

        if (pass)
            std::cout << "Test " << testCounter << " passed (" << description << ")\n";
        else
            std::cout << "Test " << testCounter << " failed (" << description << ")\n";
    }
}

void testFENParser()
{
    Position p;
    std::string fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq e3 1 2");
    getPositionFromFEN(p, fen);
    printPosition(std::cout, p);
}