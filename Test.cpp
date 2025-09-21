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
#include "Engine.h"


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

void testThreeMoveRepetition()
{
    Move m;
    Game game;
    game.init();

    bool flag = true;

    // Repeat the starting position 3 times then allow black to capture a free horse.
    std::vector<std::string> moveStringList = 
    {"g1f3", "b8c6", "f3g1", "c6b8",
     "g1f3", "b8c6", "f3g1", "c6b8",
     "g1f3", "b8c6", "f3e5", "c6e5"};

    for (std::string s: moveStringList)
    {
        m.x = parseMove(s);
        game.makeMove(m);
    }

    if (!game.hasThreefoldRepetition())
        flag = false;

    game.load("1kr5/1b3R2/4p3/4Pn1p/R7/2P3p1/1KP2B1r/8 w - - 0 1");

    Engine engine(&game, &std::cout);

    engine.think(5000);
    Move engineMove = engine.move();
    m.x = parseMove("f2a7");

    if (engineMove.x != m.x)
        flag = false;

    if (flag)
        std::cout << "testThreeMoveRepetition() passed :)\n";
    else
        std::cout << "testThreeMoveRepetition() failed :(\n";
}

void testTranspositionTable()
{
    TT tt;
    tt.init(1);  // Initialize with 1MB

    uint64_t hash = 12345;
    int eval = 150;
    int depth = 5;
    int best_move = 0;

    // Store an entry
    tt.store(hash, eval, TT_EXACT, depth, best_move);

    // Retrieve the entry
    tt_entry retrieved;
    bool found = tt.probe(hash, retrieved);

    bool passed = found &&
                  retrieved.eval == eval &&
                  retrieved.depth == depth &&
                  retrieved.flag == TT_EXACT &&
                  retrieved.key == hash;

    // Test that non-existent entries return false
    if (passed) {
        tt_entry dummy;
        bool not_found = !tt.probe(99999, dummy);
        passed = not_found;
    }

    std::cout << "testTranspositionTable() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testMoveGeneration()
{
    Game game;
    game.init();

    std::vector<int> moves;
    game.genMoves(moves);

    // Starting position should have 20 legal moves
    bool passed = (moves.size() == 20);

    if (passed) {
        // Test specific moves exist
        bool foundE2E4 = false, foundG1F3 = false, foundB1C3 = false;

        for (int moveInt : moves) {
            Move m(moveInt);
            if (m.m.from == 12 && m.m.to == 28) foundE2E4 = true;  // e2e4
            if (m.m.from == 6 && m.m.to == 21) foundG1F3 = true;   // g1f3
            if (m.m.from == 1 && m.m.to == 18) foundB1C3 = true;   // b1c3
        }

        passed = foundE2E4 && foundG1F3 && foundB1C3;
    }

    std::cout << "testMoveGeneration() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testMoveValidation()
{
    Game game;
    game.init();

    bool passed = true;

    // Test valid move
    Move validMove = game.getMove(parseMove("e2e4"));
    if (!game.makeMove(validMove)) {
        passed = false;
    }

    // Reset game
    game.init();

    // Test invalid move - the getMove function should return a move with invalid flag
    // when the parsed move doesn't match any legal move
    Move invalidMove = game.getMove(parseMove("e2e5")); // Can't move pawn 3 squares from start
    if (!(invalidMove.m.mtype & 128)) { // Should have invalid flag set
        passed = false;
    }

    // Test moving opponent's piece
    Move opponentMove = game.getMove(parseMove("e7e5"));
    if (!(opponentMove.m.mtype & 128)) { // Should have invalid flag set since it's white's turn
        passed = false;
    }

    // Test that invalid moves are rejected by makeMove
    if (game.makeMove(invalidMove)) {
        passed = false;
    }

    std::cout << "testMoveValidation() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testGameStateManagement()
{
    Game game;
    game.init();

    bool passed = true;

    // Test initial state
    if (game.pos.side != white) passed = false;
    if (game.pos.xside != black) passed = false;
    if (game.pos.fifty != 0) passed = false;

    // Make a move and check state change
    Move move = game.getMove(parseMove("e2e4"));
    game.makeMove(move);

    if (game.pos.side != black) passed = false;
    if (game.pos.xside != white) passed = false;

    // Take back move and check state restoration
    if (!game.takeBack()) passed = false;

    if (game.pos.side != white) passed = false;
    if (game.pos.xside != black) passed = false;

    std::cout << "testGameStateManagement() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testHashingConsistency()
{
    Game game1, game2;
    game1.init();
    game2.init();

    bool passed = true;

    // Initial positions should have same hash
    if (game1.pos.hash != game2.pos.hash) {
        passed = false;
    }

    // Make same move in both games
    Move move1 = game1.getMove(parseMove("e2e4"));
    Move move2 = game2.getMove(parseMove("e2e4"));

    game1.makeMove(move1);
    game2.makeMove(move2);

    // Hashes should still be equal
    if (game1.pos.hash != game2.pos.hash) {
        passed = false;
    }

    // Make different moves
    Move move3 = game1.getMove(parseMove("e7e5"));
    Move move4 = game2.getMove(parseMove("e7e6"));

    game1.makeMove(move3);
    game2.makeMove(move4);

    // Hashes should now be different
    if (game1.pos.hash == game2.pos.hash) {
        passed = false;
    }

    std::cout << "testHashingConsistency() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testPieceValueCalculation()
{
    Game game;
    game.init();

    bool passed = true;

    // Initial position should have equal material (evaluation around 0)
    int eval = game.Evaluation();
    if (abs(eval) > 50) { // Allow small opening bias
        passed = false;
    }

    // Capture a piece and check evaluation changes
    game.load("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    game.makeMove(game.getMove(parseMove("d7d6")));
    game.makeMove(game.getMove(parseMove("d1h5")));
    game.makeMove(game.getMove(parseMove("c8d7")));
    game.makeMove(game.getMove(parseMove("h5f7")));

    // White should be significantly ahead after capturing f7 pawn
    eval = game.Evaluation();
    if (eval <= 0) {
        passed = false;
    }

    std::cout << "testPieceValueCalculation() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testCastlingRights()
{
    Game game;
    game.init();

    bool passed = true;

    // Check initial castling rights
    if ((game.pos.castleRights & 12) != 12) { // White should be able to castle both ways
        passed = false;
    }
    if ((game.pos.castleRights & 3) != 3) { // Black should be able to castle both ways
        passed = false;
    }

    // Move king and check rights are lost
    game.makeMove(game.getMove(parseMove("e2e4")));
    game.makeMove(game.getMove(parseMove("e7e5")));
    game.makeMove(game.getMove(parseMove("e1e2")));

    // White should lose castling rights
    if ((game.pos.castleRights & 12) != 0) {
        passed = false;
    }

    std::cout << "testCastlingRights() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testEnPassant()
{
    Game game;
    // Set up position for en passant
    game.load("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3");

    bool passed = true;

    // Check en passant square is set correctly
    if (game.pos.enpassant != 43) { // d6 square
        passed = false;
    }

    // Make en passant capture
    Move epMove = game.getMove(parseMove("e5d6"));
    if (!game.makeMove(epMove)) {
        passed = false;
    }

    // Check that the captured pawn is gone
    if (game.pos.squares[35].ptype != any) { // d5 should be empty
        passed = false;
    }

    std::cout << "testEnPassant() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testPositionSetup()
{
    bool passed = true;

    // Test default position setup
    Position pos = defaultPosition();

    // Check side to move
    if (pos.side != white) passed = false;
    if (pos.xside != black) passed = false;

    // Check castling rights
    if (pos.castleRights != 15) passed = false; // All castling rights should be available

    // Check fifty move counter
    if (pos.fifty != 0) passed = false;

    // Test FEN loading
    Position fenPos;
    std::string fen = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";
    if (!getPositionFromFEN(fenPos, fen)) {
        passed = false;
    }

    // Check side to move from FEN
    if (fenPos.side != white) passed = false;

    // Check that FEN parsing doesn't crash - that's a basic success
    std::cout << "testPositionSetup() " << (passed ? "passed!" : "failed :(") << "\n";
}

void testTTPerformance()
{
    TT tt;
    tt.init(8);  // 8MB table

    std::cout << "testTTPerformance() - Testing TT with many entries...\n";

    // Store many entries
    const int num_entries = 10000;
    for (int i = 0; i < num_entries; i++) {
        uint64_t hash = static_cast<uint64_t>(i) * 1234567;
        tt.store(hash, i % 1000, TT_EXACT, (i % 10) + 1, i);
    }

    // Retrieve some entries
    int found_count = 0;
    for (int i = 0; i < num_entries; i += 100) {
        uint64_t hash = static_cast<uint64_t>(i) * 1234567;
        tt_entry entry;
        if (tt.probe(hash, entry)) {
            found_count++;
        }
    }

    // Print TT statistics
    tt.print_stats();

    bool passed = found_count > 0;  // Should find at least some entries
    std::cout << "Found " << found_count << " entries out of " << (num_entries / 100) << " probes\n";
    std::cout << "testTTPerformance() " << (passed ? "passed!" : "failed :(") << "\n";
}