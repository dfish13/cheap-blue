#ifndef HASH_H
#define HASH_H 

#include "Defs.h"
#include <random>

#define SEED 5

class Hash
{

public:
  Hash();
  void operator()(Position & p);

private:

  // One value for each combination of piece type and square
  long hashPieceSquare[12][64];
  // One value for each color, white or black
  long hashSide[2];
  // One value for each combination of castling rights.
  // There are 4 possible castlings so 2 ^ 4 = 16 combinations.
  long hashCastle[16];
  // One value for each of the 8 files for which the
  // enpassant square could be active.
  long hashEp[8];
};

#endif