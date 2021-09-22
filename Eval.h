#ifndef EVAL_H
#define EVAL_H

#include "Defs.h"

#define DOUBLED_PAWN_PENALTY		10
#define ISOLATED_PAWN_PENALTY		20
#define BACKWARDS_PAWN_PENALTY		8
#define PASSED_PAWN_BONUS			20
#define ROOK_SEMI_OPEN_FILE_BONUS	10
#define ROOK_OPEN_FILE_BONUS		15
#define ROOK_ON_SEVENTH_BONUS		20

namespace Eval {

int eval(Position p);
int eval_white_pawn(int sq);
int eval_black_pawn(int sq);
int eval_white_king(int sq);
int eval_wkp(int f);
int eval_black_king(int sq);
int eval_bkp(int f);

}

#endif