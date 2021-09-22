#ifndef UTIL_H
#define UTIL_H

#include "Defs.h"

#include <iostream>
#include <sstream>
#include <list>

int parseSquare(std::string s);
int parseMove(std::string m);
int parseFEN(std::string f);

std::string indexToSquare(int i);
std::string getMoveString(Move m);

// Returns a Piece object given a character in the set "PNBRQKpnbrqk"
char getCharacterFromPiece(Piece p);
Piece getPieceFromCharacter(char c);


// Two ways to initialize a Position object
Position defaultPosition();
bool getPositionFromFEN(Position & pos, std::string fen);    // Returns -1 if invalid FEN string

void printBoard(std::ostream & os, const Position & p);

// Print values in a Position object. For testing/debugging.
void printPosition(std::ostream & os, const Position & p);

bool checkPosition(const Position & p, std::ostream & os);

#endif