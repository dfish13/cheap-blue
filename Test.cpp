#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include "Defs.h"
#include "Util.h"
#include "Game.h"
#include "Test.h"
#include "Eval.h"


void testEval()
{
    Game game;
    game.init();

    if (Eval::eval(game.pos) == 0)
        std::cout << "testEval() passed!\n";
    else
        std::cout << "testEval() failed :(\n";
}

/**
 * All of these positions and perft values were found at:
 * https://www.chessprogramming.org/Perft_Results
 * 
 * WARNING!! This test make take a while to run.
 */
void testPerft()
{
    std::vector<PerftTest> perftTests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4, 4085603},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333},
        {"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", 4, 422333}
    };

    Game game;
    game.init();
    long nodes;
    int testN = 1;

    for (PerftTest p: perftTests)
    {
        game.load(p.fen);
        nodes = game.Perft(p.depth);
        if (nodes != p.nodes)
            std::cout << "Perft test " << testN << " failed :( ";
        else
            std::cout << "Perft test " << testN << " passed! ";
        std::cout << nodes << ' ' << p.nodes << '\n';
        ++testN;
    }

}

void testBoardIsAttacked()
{
    Move m;
    Game game;
    game.init();

    bool flag = false;

    if (!game.isAttacked(20, white))
        flag = true;
    if (!game.isAttacked(21, white))
        flag = true;
    m.x = parseMove("e2e4");
    game.makeMove(m);

    if (!game.isAttacked(35, white))
        flag = true;
    if (!game.isAttacked(39, white))
        flag = true;
    if (!game.isAttacked(40, white))
        flag = true;
    m.x = parseMove("b8c6");
    game.makeMove(m);

    if (!game.isAttacked(27, black))
        flag = true;
    if (game.isAttacked(28, black))
        flag = true;

    std::cout << "testBoardIsAttacked() " << ((flag) ? "failed :(\n" : "passed!\n");
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