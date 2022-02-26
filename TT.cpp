#include "TT.h"

TT::TT() {}

void TT::put(long hash, tt_entry e)
{
  auto got = table.find(hash);
  if (got == table.end() || (got->second).depth < e.depth)
    table[hash] = e;
}

bool TT::get(long hash, tt_entry & e)
{
  auto got = table.find(hash);
  if (got != table.end())
  {
    e = (got->second);
    return true;
  }
  return false;
}