#ifndef TEST_H
#define TEST_H

struct PerftTest
{
    std::string fen;
    int depth;
    long nodes;
};


void testEval();
void testPerft();
void testBoardIsAttacked();
void testIntegration();
void testFENParser();
void testInCheck();
void testThreeMoveRepetition();
void testTranspositionTable();

// New unit tests
void testMoveGeneration();
void testMoveValidation();
void testGameStateManagement();
void testHashingConsistency();
void testPieceValueCalculation();
void testCastlingRights();
void testEnPassant();
void testPositionSetup();
void testTTPerformance();

// Stress tests
void testStress();

#endif