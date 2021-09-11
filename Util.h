#ifndef UTIL_H
#define UTIL_H

#include "Defs.h"

#include <string>
#include <ostream>

int parseSquare(std::string s);
int parseMove(std::string m);
int parseFEN(std::string f);

// Returns a Piece object given a character in the set "PNBRQKpnbrqk"
char getCharacterFromPiece(Piece p);
Piece getPieceFromCharacter(char c);


// Two ways to initialize a Position object
Position defaultPosition();
Position getPositionFromFEN(std::string fen);

void printPosition(std::ostream & os, Position p);

#endif