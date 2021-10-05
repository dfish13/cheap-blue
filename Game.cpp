#include "Game.h"

void Game::init()
{
	pos = defaultPosition();
	pastMoves.reserve(PASTMOVES_STACK);
}

void Game::init(std::string fen)
{
	if (!getPositionFromFEN(pos, fen))
		pos = defaultPosition();
	pastMoves.reserve(PASTMOVES_STACK);
}

void Game::load(std::string fen)
{
	Position p;
	if (getPositionFromFEN(p, fen))
	{
		pos = p;
		pastMoves.clear();
	}
}

Move Game::getMove(int m) const
{
	Move move, imove;
	imove.x = m;
	std::vector<int> moves;
	moves.reserve(40);
	genMoves(moves);
	
	for (int x: moves)
	{
		move.x = x;

		// if move is a castle then we need to check what side (long or short).
		if (move.m.mtype & 32)
		{
			if (imove.m.mtype & 32 && move.m.detail == imove.m.detail)
				return move;
		}

		// if the from and to squares match up then we can infer this is the correct move
		else if (imove.m.from == move.m.from && imove.m.to == move.m.to && imove.m.detail == move.m.detail)
			return move; 
	}

	move.m = {128, 0, 0, 0};
	return move;
}

bool Game::makeMove(Move m)
{
	if (m.m.mtype & 128)
		return false;
	
	MoveInfo moveInfo;
	moveInfo.m = m;
	moveInfo.capture = pos.squares[m.m.to].ptype;
	moveInfo.castleRights = pos.castleRights;
	moveInfo.ep = pos.enpassant;
	moveInfo.fifty = pos.fifty;
	moveInfo.hash = pos.hash;
	pastMoves.push_back(moveInfo);

	// unset enpassant
	pos.enpassant = NSQUARES;

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
		pos.squares[m.m.to] = pos.squares[m.m.from];
		pos.squares[m.m.from] = {none, any};
	}

	pos.castleRights &= castle_mask[m.m.to] & castle_mask[m.m.from]; 
	
	if (m.m.mtype)
		pos.fifty = 0;
	else
		++pos.fifty;
	std::swap(pos.side, pos.xside);

	if (inCheck(pos.xside))
	{
		takeBack();
		return false;
	}
	setHash();

	return true;
}

bool Game::takeBack()
{
	if (pastMoves.empty())
		return false;
	MoveInfo mInfo = pastMoves[pastMoves.size() - 1];
	Move m;

	std::swap(pos.side, pos.xside);
	pastMoves.pop_back();
	m = mInfo.m;
	pos.castleRights = mInfo.castleRights;
	pos.enpassant = mInfo.ep;
	pos.fifty = mInfo.fifty;
	pos.hash = mInfo.hash;
	pos.squares[m.m.from].color = pos.side;
	
	if (m.m.mtype & 16) // Pawn promotion so change back to pawn.
		pos.squares[m.m.from].ptype = pawn;
	else
		pos.squares[m.m.from].ptype = pos.squares[m.m.to].ptype;
	
	if (mInfo.capture == any) // Restore captured piece.
		pos.squares[m.m.to] = {none, any};
	else
		pos.squares[m.m.to] = {pos.xside, mInfo.capture};
	
	if (m.m.mtype & 32) // Restore rook for a castling move.
	{
		int from, to;
		switch (m.m.to)
		{
			case G1:
				from = F1;
				to = H1;
				break;
			case C1:
				from = D1;
				to = A1;
				break;
			case G8:
				from = F8;
				to = H8;
				break;
			case C8:
				from = D8;
				to = A8;
				break;
			default:
				from = to = 0;
				break;
		}
		pos.squares[to] = {pos.side, rook};
		pos.squares[from] = {none, any};
	}

	if (m.m.mtype & 2) // Enpassant capture.
	{
		/*
			Just to document an awful bug which caused me much
			pain and suffering, the following line used to be:
		if (pos.side = white)
		*/
		if (pos.side == white)
			pos.squares[m.m.to - 8] = {pos.xside, pawn};
		else
			pos.squares[m.m.to + 8] = {pos.xside, pawn};
	}

	return true;
}

bool Game::isAttacked(int square, Color c) const
{
	for (int i = 1; i <= king; ++i)
	{
		for(int j = 0; j < offsets[i]; ++j)
		{
			for (int n = square;;)
			{
				n = mailbox[mailbox64[n] + offset[i][j]];
				if (n == -1) break;
				if (pos.squares[n].color != none)
				{
					if (pos.squares[n].color == c && pos.squares[n].ptype == i)
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

bool Game::inCheck(Color c) const
{
	for (int i = 0; i < NSQUARES; ++i)
	{
		if (pos.squares[i].color == c && pos.squares[i].ptype == king)
			return isAttacked(i, (c == white) ? black : white);
	}
	// Should not reach here, because that means there is no king
	return false;
}

void Game::genMoves(std::vector<int> & moves) const
{
	Move move;
	int promotionMoves[4];

	Piece p;
	for (uint8_t i = 0; i < 64; ++i)
	{
		if (pos.squares[i].color == pos.side)
		{
			p = pos.squares[i];
			if (p.ptype != pawn)
			{
				for (int j = 0; j < offsets[p.ptype]; ++j)
				{
					for (int n = i;;)
					{
						n = mailbox[mailbox64[n] + offset[p.ptype][j]];
						if (n == -1) break;
						if (pos.squares[n].color != none)
						{
							if (pos.squares[n].color == pos.xside)
							{
								// capture
								move.m = {64, i, static_cast<uint8_t>(n), 0};
								moves.push_back(move.x);
							}
							break;
						}
						move.m = {0, i, static_cast<uint8_t>(n), 0};
						moves.push_back(move.x);
						if (!slide[p.ptype]) break;
					}
				}
			}
			else // pawn moves
			{
				int n, m = (pos.side == white) ? 1 : -1;
				uint8_t singlePawnMove = static_cast<uint8_t>(i + m * 8);
				uint8_t doublePawnMove = static_cast<uint8_t>(i + m * 16);
				bool pawnPromotionWhite = pos.side == white && (ROW(i) == 6 );
				bool pawnPromotionBlack = pos.side == black && (ROW(i) == 1 );
				if (pos.squares[singlePawnMove].color == none)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						generatePawnPromotionMoves(i, singlePawnMove, promotionMoves);
						for (int j = 0; j < 4; ++j)
                        	moves.push_back(promotionMoves[j]);
                    }
					else
					{
						move.m = {4, i, singlePawnMove, 0};
						moves.push_back(move.x);
					}
                    
                    //double pawn for white 
					if (pos.side == white && (ROW(i) == 1) && pos.squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.push_back(move.x);
					}
                    //double pawn for black
					if (pos.side == black && (ROW(i) == 6) && pos.squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.push_back(move.x);
					}

				}
				//pawn capture square
				n = mailbox[mailbox64[i] + m * -9];
				if (n != -1 && pos.squares[n].color == pos.xside)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						generatePawnPromotionMoves(i, n, promotionMoves);
						for (int j = 0; j < 4; ++j)
							moves.push_back(promotionMoves[j]);
                    }
					else 
					{
						move.m = {64, i, static_cast<uint8_t>(n), 0};
						moves.push_back(move.x);
					}
				}
				if (n == pos.enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.push_back(move.x);
				}
				// pawn capture square
				n = mailbox[mailbox64[i] + m * -11];
				if (n != -1 && pos.squares[n].color == pos.xside)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						generatePawnPromotionMoves(i, n, promotionMoves);
                        for (int j = 0; j < 4; ++j)
							moves.push_back(promotionMoves[j]);
                    }
					else 
					{
						move.m = {64, i, static_cast<uint8_t>(n), 0};
						moves.push_back(move.x);
					}
				}
				if (n == pos.enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.push_back(move.x);
				}
			}
		}
	}

	// castling moves
	uint8_t rshift = (pos.side == white) ? 0: 56;
	uint8_t b = (pos.side == white) ? 4: 1;

	// Queenside (long) castle
	if ((pos.castleRights & (b << 1)) &&
		(pos.squares[B1 + rshift].color == none) &&
		(pos.squares[C1 + rshift].color == none) &&
		(pos.squares[D1 + rshift].color == none) &&
		!isAttacked(C1 + rshift, pos.xside) &&
		!isAttacked(D1 + rshift, pos.xside) &&
		!isAttacked(E1 + rshift, pos.xside))
	{
		move.m = {32, static_cast<uint8_t>(E1 + rshift), static_cast<uint8_t>(C1 + rshift), 1};
		moves.push_back(move.x);
	}

	// Kingside (short) castle.
	if ((pos.castleRights & b) &&
		(pos.squares[F1 + rshift].color == none) &&
		(pos.squares[G1 + rshift].color == none) &&
		!isAttacked(E1 + rshift, pos.xside) &&
		!isAttacked(F1 + rshift, pos.xside) &&
		!isAttacked(G1 + rshift, pos.xside))
	{
		move.m = {32, static_cast<uint8_t>(E1 + rshift), static_cast<uint8_t>(G1 + rshift), 2};
		moves.push_back(move.x);
	}
}

void Game::genCaptures(std::vector<int> & moves) const
{
	Move move;
	int promotionMoves[4];

	Piece p;
	for (uint8_t i = 0; i < 64; ++i)
	{
		if (pos.squares[i].color == pos.side)
		{
			p = pos.squares[i];
			if (p.ptype == pawn)
			{
				if (pos.side == white)
				{
					if (COL(i) != 0 && pos.squares[i + 7].color == black)
					{
						if (ROW(i) == 6)
						{
							generatePawnPromotionMoves(i, i + 7, promotionMoves);
                        	for (int j = 0; j < 4; ++j)
								moves.push_back(promotionMoves[j]);
						}
						else
						{
							move.m = {64, i, static_cast<uint8_t>(i + 7), 0};
							moves.push_back(move.x);
						}
						
					}
					if (COL(i) != 7 && pos.squares[i + 9].color == black)
					{
						if (ROW(i) == 6)
						{
							generatePawnPromotionMoves(i, i + 9, promotionMoves);
                        	for (int j = 0; j < 4; ++j)
								moves.push_back(promotionMoves[j]);
						}
						else
						{
							move.m = {64, i, static_cast<uint8_t>(i + 9), 0};
							moves.push_back(move.x);
						}
					}
					if ((ROW(i) == 6) && pos.squares[i + 8].color == none)
					{
						generatePawnPromotionMoves(i, i + 8, promotionMoves);
                        for (int j = 0; j < 4; ++j)
							moves.push_back(promotionMoves[j]);
					}
				}
				if (pos.side == black)
				{
					if (COL(i) != 0 && pos.squares[i - 9].color == white)
					{
						if (ROW(i) == 1)
						{
							generatePawnPromotionMoves(i, i - 9, promotionMoves);
                        	for (int j = 0; j < 4; ++j)
								moves.push_back(promotionMoves[j]);
						}
						else
						{
							move.m = {64, i, static_cast<uint8_t>(i - 9), 0};
							moves.push_back(move.x);
						}
					}
					if (COL(i) != 7 && pos.squares[i - 7].color == white)
					{
						if (ROW(i) == 1)
						{
							generatePawnPromotionMoves(i, i - 7, promotionMoves);
                        	for (int j = 0; j < 4; ++j)
								moves.push_back(promotionMoves[j]);
						}
						else
						{
							move.m = {64, i, static_cast<uint8_t>(i - 7), 0};
							moves.push_back(move.x);
						}
					}
					if (i <= 15 && pos.squares[i - 8].color == none)
					{
						generatePawnPromotionMoves(i, i - 8, promotionMoves);
                        for (int j = 0; j < 4; ++j)
							moves.push_back(promotionMoves[j]);
					}
				}
			}
			else
			{
				for (int j = 0; j < offsets[p.ptype]; ++j)
				{
					for (int n = i;;)
					{
						n = mailbox[mailbox64[n] + offset[p.ptype][j]];
						if (n == -1) break;
						if (pos.squares[n].color != none)
						{
							if (pos.squares[n].color == pos.xside)
							{
								// capture
								move.m = {64, i, static_cast<uint8_t>(n), 0};
								moves.push_back(move.x);
							}
							break;
						}
						if (!slide[p.ptype]) break;
					}
				}
			}
		}
	}

	if (pos.enpassant < NSQUARES)
	{
		if (pos.side == white)
		{
			if (COL(pos.enpassant) != 0 && Piece({white, pawn}) == pos.squares[pos.enpassant - 9])
			{
				move.m = {2, static_cast<uint8_t>(pos.enpassant - 9), static_cast<uint8_t>(pos.enpassant), 0};
				moves.push_back(move.x);
			}
			if (COL(pos.enpassant) != 7 && Piece({white, pawn}) == pos.squares[pos.enpassant - 7])
			{
				move.m = {2, static_cast<uint8_t>(pos.enpassant - 7), static_cast<uint8_t>(pos.enpassant), 0};
				moves.push_back(move.x);
			}
		}
		else
		{
			if (COL(pos.enpassant) != 0 && Piece({black, pawn}) == pos.squares[pos.enpassant + 7])
			{
				move.m = {2, static_cast<uint8_t>(pos.enpassant + 7), static_cast<uint8_t>(pos.enpassant), 0};
				moves.push_back(move.x);
			}
			if (COL(pos.enpassant) != 7 && Piece({black, pawn}) == pos.squares[pos.enpassant + 9])
			{
				move.m = {2, static_cast<uint8_t>(pos.enpassant + 9), static_cast<uint8_t>(pos.enpassant), 0};
				moves.push_back(move.x);
			}
		}
	}
}

std::vector<int> Game::genLegalMoves()
{
	std::vector<int> legalmoves, moves;
	genMoves(moves);
	Move m;

	legalmoves.reserve(moves.size());
	for (int x: moves)
	{
		m.x = x;
		if (makeMove(m))
		{
			legalmoves.push_back(m.x);
			takeBack();
		}	
	}

	return legalmoves;
}

void Game::generatePawnPromotionMoves(uint8_t from, uint8_t to, int * promotionMoves) 
{
	Move promotionMove;
	int j = 0;
	// 1,2,4,8 represent N,B,R,Q
	for(uint8_t i = 1; i <=8; i<<=1)
	{
		promotionMove.m = {16, from, to, i};
		promotionMoves[j++] = promotionMove.x;
	}
}

void Game::setHash()
{
	pos.hash = 0;
}

/*
	Same display board function as TSCP.
	TSCP was the first chess engine I examined so as I develop my own
	engine, I'm sure I will get a lot of help and inspiration from TSCP.

	http://www.tckerrigan.com/Chess/TSCP/
*/
void Game::display(std::ostream & os) const
{
	printBoard(os, pos);
}

Move Game::getRandomMove()
{
	Move m;
	std::vector<int> moves = genLegalMoves();
	if (moves.empty())
		m.m = {128, 0, 0, 0};
	else
	{
		srand(time(NULL));
		m.x = moves[rand() % moves.size()];
	}
	return m;
}

long Game::Perft(int depth)
{
	if (depth == 0)
		return 1;
	long nodes = 0;
	Move m;
	std::vector<int> moves;
	moves.reserve(40);
	genMoves(moves);
	for (int x: moves)
	{
		m.x = x;
		if (makeMove(m))
		{
			nodes += Perft(depth - 1);
			takeBack();
		}
	}
	return nodes;

}

double Game::Evaluation()
{
	double score;
	score = (double) Eval::eval(pos);
	score /= 100.0;
	return (pos.side == white) ? score : -score;
}

int Game::eval()
{
	return Eval::eval(pos);
}
