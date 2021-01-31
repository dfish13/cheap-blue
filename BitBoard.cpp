#include "BitBoard.h"

long BitBoard::mask(int i)
{
	long m = 1;
	return m << (63 - i);
}

bool BitBoard::isOccupied(int i) const
{
	return mask(i) & bb;
}
