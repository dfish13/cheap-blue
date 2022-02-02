#include "Hash.h"

Hash::Hash()
{
  std::mt19937_64 gen(SEED);
  for (int i = 0; i < 12; ++i)
    for (int j = 0; j < 64; ++j)
      hashPieceSquare[i][j] = gen();
  hashSide[0] = gen();
  hashSide[1] = gen();
  for (int i = 0; i < 16; ++i)
    hashCastle[i] = gen();
  for (int i = 0; i < 8; ++i)
    hashEp[i] = gen();
}

void Hash::operator()(Position & p)
{
  long hash = 0;
  for (int i = 0; i < 64; ++i)
    if (p.squares[i].color < 2)
      hash ^= hashPieceSquare[p.squares[i].color * 6 + p.squares[i].ptype][i];
  hash ^= hashCastle[p.castleRights];
  hash ^= hashSide[p.side];
  if (p.enpassant < 64)
    hash ^= hashEp[COL(p.enpassant)];
  p.hash = hash;
}