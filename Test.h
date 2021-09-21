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

#endif