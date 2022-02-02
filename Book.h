#ifndef BOOK_H
#define BOOK_H

#include "Defs.h"
#include "Game.h"
#include "Util.h"

#include <unordered_map>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>

class Book
{
public:
  Book();
  void init(const char bookFile[]);
  bool getMove(long hash, Move & m);

private:
  std::unordered_map<long, std::vector<int>> moves;
  std::minstd_rand0 gen; // psuedo-random number generator
};


#endif