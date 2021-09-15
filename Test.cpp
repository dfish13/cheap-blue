#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "Defs.h"
#include "Util.h"
#include "Game.h"
#include "Test.h"

void testBoardIsAttacked()
{
    Move m;
    Game game;
    game.init();


    std::cout << game.isAttacked(20, white) << std::endl;
    std::cout << game.isAttacked(21, white) << std::endl;
    m.x = parseMove("e2e4");
    game.makeMove(m);

    std::cout << game.isAttacked(35, white) << std::endl;
    std::cout << game.isAttacked(39, white) << std::endl;
    std::cout << game.isAttacked(40, white) << std::endl;
    m.x = parseMove("b8c6");
    game.makeMove(m);

    std::cout << game.isAttacked(27, black) << std::endl;
    std::cout << game.isAttacked(28, black) << std::endl;

}

void testIntegration()
{  

    std::string s, tok, description;
    std::ifstream fin;
    fin.open("tests");

    int testCounter = 0;

    Move move;
	Game game;
    bool pass;
	

    while (getline(fin, s))
    {
        pass = true;
        game.init();
        ++testCounter;
        std::istringstream iss(s);
        
        iss >> description;
        while (iss >> tok)
        {
            move = game.getMove(parseMove(tok));
            if (!game.makeMove(move))
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

void testInCheck()
{
    Move m;
    Game game;
    game.init();
    bool flag = true;


    if (game.inCheck(white))
        flag = false;
    if (game.inCheck(black))
        flag = false;
    m = game.getMove(parseMove("e2e4"));
    game.makeMove(m);
    if (game.inCheck(white))
        flag = false;
    if (game.inCheck(black))
        flag = false;
    m = game.getMove(parseMove("e7e5"));
    game.makeMove(m);
    m = game.getMove(parseMove("d1f3"));
    game.makeMove(m);
    m = game.getMove(parseMove("b8c6"));
    game.makeMove(m);
    if (game.inCheck(white))
        flag = false;
    if (game.inCheck(black))
        flag = false;
    m = game.getMove(parseMove("f3f7"));
    game.makeMove(m);
    if (game.inCheck(white))
        flag = false;
    if (!game.inCheck(black)) // Black should be in check
        flag = false;
    

    if (flag)
        std::cout << "testInCheck() passed :)\n";
    else
        std::cout << "testInCheck() failed :(\n";
}