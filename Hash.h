#include "Defs.h"


class Hash
{

public:
  Hash();
  void operator()(Position & p);

private:
  long hash;
};