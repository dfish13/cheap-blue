#include "Board.h"
#include "Util.h"



void Board::init()
{
	pos = defaultPosition();
}

void Board::init(std::string fen)
{
	pos = getPositionFromFEN(fen);
}

Move Board::getMove(int m) const
{
	Move move, imove;
	imove.x = m;
	set<int> moves = genMoves();
	
	if (imove.m.mtype)
	{
		if (moves.find(imove.x) != moves.end())
			return imove;
		else
		{
			move.m = {128, 0, 0, 0};
			return move;
		}
	}
	
	for (int x: moves)
	{
		move.x = x;
		// if the from and to squares match up then we can infer this is the correct move
		if (imove.m.from == move.m.from && imove.m.to == move.m.to && imove.m.detail == move.m.detail)
			return move; 
	}

	move.m = {128, 0, 0, 0};
	return move;
}

bool Board::makeMove(Move m)
{
	if (!(m.m.mtype & 128))
	{
		// unset enpassant
		// Although 0 is an index for the square A1 it will never be an enpassant square
		// so there is no problem.
		pos.enpassant = 0;

		int rshift = (pos.side == white) ? 0: 56;
		uint8_t b = (pos.side == white) ? 4: 1;

		if (m.m.mtype & 32)
		{
			// castling
			if (m.m.detail & 1)
			{
				pos.squares[A1 + rshift] = {none, any};
				pos.squares[E1 + rshift] = {none, any};
				pos.squares[C1 + rshift] = {pos.side, king};
				pos.squares[D1 + rshift] = {pos.side, rook};
			}
			else if (m.m.detail & 2)
			{
				pos.squares[E1 + rshift] = {none, any};
				pos.squares[H1 + rshift] = {none, any};
				pos.squares[F1 + rshift] = {pos.side, rook};
				pos.squares[G1 + rshift] = {pos.side, king};
			}

			pos.castleRights &= ~(b << 1);
			pos.castleRights &= ~b;
		}
		else if (m.m.mtype & 16)
		{
			uint8_t piece = m.m.detail;
			PType promotionPiece;
				if(piece & 1)
					promotionPiece = knight;
				else if(piece & 2)
					promotionPiece = bishop;
				else if(piece & 4)
					promotionPiece = rook;
				else if(piece & 8)
					promotionPiece = queen;
				else {
					return false;

				}
			pos.squares[m.m.to] = {pos.side, promotionPiece};
			pos.squares[m.m.from] = {none, any};
			
            
		}
		else if(m.m.mtype & 8)
		{
			// double pawn move
			pos.squares[m.m.to] = pos.squares[m.m.from];
			pos.squares[m.m.from] = {none, any};
			pos.enpassant = (m.m.to + m.m.from) / 2;
		}
		else if(m.m.mtype & 2)
		{
			// enpassant capture
			pos.squares[m.m.to] = pos.squares[m.m.from];
			pos.squares[m.m.from] = {none, any};
			if (pos.side == white)
				pos.squares[m.m.to - 8] = {none, any};
			else
				pos.squares[m.m.to + 8] = {none, any};
		}
		else
		{
			// If moving king or rook from their starting squares, lose castling rights.
			if (m.m.from == (A1 + rshift) && pos.squares[m.m.from].ptype == rook)
				pos.castleRights &= ~(b << 1);
			else if (m.m.from == (H1 + rshift) && pos.squares[m.m.from].ptype == rook)
				pos.castleRights &= ~b;
			else if (m.m.from == (E1 + rshift) && pos.squares[m.m.from].ptype == king)
			{
				pos.castleRights &= ~(b << 1);
				pos.castleRights &= ~b;
			}
			pos.squares[m.m.to] = pos.squares[m.m.from];
			pos.squares[m.m.from] = {none, any};
		}
		
		if (m.m.mtype)
			pos.fifty = 0;
		else
			++pos.fifty;
		swap(pos.side, pos.xside);
		++pos.ply;
		return true;
	}
	return false;
}

bool Board::isAttacked(int square, Color c) const
{
	for (int i = 1; i < 6; ++i)
	{
		for(int j = 0; j < offsets[i]; ++j)
		{
			for (int n = square;;)
			{
				n = mailbox[mailbox64[n] + offset[i][j]];
				if (n == -1) break;
				if (pos.squares[n].color != none)
				{
					if (pos.squares[n].color == c && (pos.squares[n].ptype - pawn) == i)
						return true;
					break;
				}
				if (!slide[i]) break;
			}
		}
	}
	
	// check if is attacked by a pawn
	int n, m = (c == white) ? 1 : -1;
	n = mailbox[mailbox64[square] + m * 9];
	if (n != -1 && pos.squares[n].color == c && pos.squares[n].ptype == pawn)
		return true;
	n = mailbox[mailbox64[square] + m * 11];
	if (n != -1 && pos.squares[n].color == c && pos.squares[n].ptype == pawn)
		return true;
	return false;
}

set<int> Board::genMoves() const
{
	set<int> moves;
	Move move;

	Piece p;
	for (uint8_t i = 0; i < 64; ++i)
	{
		if (pos.squares[i].color == pos.side)
		{
			p = pos.squares[i];
			if (p.ptype != pawn)
			{
				for (int j = 0; j < offsets[p.ptype - pawn]; ++j)
				{
					for (int n = i;;)
					{
						n = mailbox[mailbox64[n] + offset[p.ptype - pawn][j]];
						if (n == -1) break;
						if (pos.squares[n].color != none)
						{
							if (pos.squares[n].color == pos.xside)
							{
								// capture
								move.m = {64, i, static_cast<uint8_t>(n), 0};
								moves.insert(move.x);
							}
							break;
						}
						move.m = {0, i, static_cast<uint8_t>(n), 0};
						moves.insert(move.x);
						if (!slide[p.ptype - pawn]) break;
					}
				}
			}
			else // pawn moves
			{
				int n, m = (pos.side == white) ? 1 : -1;
				uint8_t singlePawnMove = static_cast<uint8_t>(i + m * 8);
				uint8_t doublePawnMove = static_cast<uint8_t>(i + m * 16);
				bool pawnPromotionWhite = pos.side == white && (i / 8 == 6 );
				bool pawnPromotionBlack = pos.side == black && (i / 8 == 1 );
				if (pos.squares[singlePawnMove].color == none)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						set<int> pawnPromotionMoves = generatePawnPromotionMoves(i, singlePawnMove);
                        moves.insert(pawnPromotionMoves.begin(), pawnPromotionMoves.end());
                    }
					else
					{
						move.m = {4, i, singlePawnMove, 0};
						moves.insert(move.x);
					}
                    
                    //double pawn for white 
					if (pos.side == white && (i / 8 == 1) && pos.squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.insert(move.x);
					}
                    //double pawn for black
					if (pos.side == black && (i / 8 == 6) && pos.squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.insert(move.x);
					}

				}
				//pawn capture square
				n = mailbox[mailbox64[i] + m * -9];
				if (n != -1 && pos.squares[n].color == pos.xside)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						set<int> pawnPromotionMoves = generatePawnPromotionMoves(i, n);
                        moves.insert(pawnPromotionMoves.begin(), pawnPromotionMoves.end());
                    }
					else 
					{
						move.m = {64, i, static_cast<uint8_t>(n), 0};
						moves.insert(move.x);
					}
				}
				if (n == pos.enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.insert(move.x);
				}
				//pawn capture square
				n = mailbox[mailbox64[i] + m * -11];
				if (n != -1 && pos.squares[n].color == pos.xside)
				{
                    if(pawnPromotionBlack || pawnPromotionBlack)
                    {
						set<int> pawnPromotionMoves = generatePawnPromotionMoves(i, n);
                        moves.insert(pawnPromotionMoves.begin(), pawnPromotionMoves.end());
                    }
					else 
					{
						move.m = {64, i, static_cast<uint8_t>(n), 0};
						moves.insert(move.x);
					}
				}
				if (n == pos.enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.insert(move.x);
				}
			}

		}
	}

	// castling moves
	int rshift = (pos.side == white) ? 0: 56;
	uint8_t b = (pos.side == white) ? 4: 1;
	if ((pos.castleRights & (b << 1)) &&
		(pos.squares[B1 + rshift].color == none) &&
		(pos.squares[C1 + rshift].color == none) &&
		(pos.squares[D1 + rshift].color == none) &&
		!isAttacked(C1 + rshift, pos.xside) &&
		!isAttacked(D1 + rshift, pos.xside) &&
		!isAttacked(E1 + rshift, pos.xside))
	{
		move.m = {32, 0, 0, 1};
		moves.insert(move.x);
	}
	if ((pos.castleRights & b) &&
		(pos.squares[F1 + rshift].color == none) &&
		(pos.squares[G1 + rshift].color == none) &&
		!isAttacked(E1 + rshift, pos.xside) &&
		!isAttacked(F1 + rshift, pos.xside) &&
		!isAttacked(G1 + rshift, pos.xside))
	{
		move.m = {32, 0, 0, 2};
		moves.insert(move.x);
	}


	return moves;
}

set<int> Board::generatePawnPromotionMoves(uint8_t from, uint8_t to) 
{

	set<int> promotionMoves;
	Move promotionMove;
	
	//1,2,4,8 represent N,B,R,Q
	for(uint8_t i = 1; i <=8; i<<=1)
	{
		promotionMove.m = {16, from, to, i};
		promotionMoves.insert(promotionMove.x);
	}
	return promotionMoves;
}

/*
	Same display board function as TSCP.
	TSCP was the first chess engine I examined so as I develop my own
	engine, I'm sure I will get a lot of help and inspiration from TSCP.

	http://www.tckerrigan.com/Chess/TSCP/
*/
void Board::display(ostream & os) const
{
	printPosition(os, pos);
}
