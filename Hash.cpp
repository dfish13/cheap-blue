#include "Hash.h"

Hash::Hash() { hash = 0; }

void Hash::operator()(Position & p) { p.hash = hash; }