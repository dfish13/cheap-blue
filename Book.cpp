#include "Book.h"

Book::Book() {}

void Book::init(const char bookFile[])
{
  std::ifstream fin(bookFile);
  std::string s;
  Game g;
  Move m;
  g.init();

  // Fill table with moves
  while (fin)
  {
    fin >> s;
    if (s == "~")
      g.takeBack();
    else
    {
      m = g.getMove(parseMove(s));
      moves[g.pos.hash].push_back(m.x);
      g.makeMove(m);
    }
  }
  fin.close();

  // seed pseudo-random number generator with current time.
  gen.seed(time(0));
}

bool Book::getMove(long hash, Move & m)
{
  if (moves.count(hash) == 0)
    return false;
  auto & moveList = moves[hash];
  m.x = moveList[gen() % moveList.size()];
  return true;

}