#ifndef TT_H
#define TT_H

#include <unordered_map>

struct tt_entry
{
  int eval;
  int depth;
};


/**
 * @brief Transposition Table
 * 
 */
class TT
{
public:
  TT();
  void put(long hash, tt_entry);
  bool get(long hash, tt_entry & e);

private:
  std::unordered_map<long, tt_entry> table;
};

#endif