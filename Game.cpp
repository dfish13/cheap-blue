#include "Game.h"
#include "Util.h"


void Game::init()
{
	pos = defaultPosition();
	movestack.reserve(MOVE_STACK);
	history.reserve(HIST_STACK);
}

void Game::init(std::string fen)
{
	if (!getPositionFromFEN(pos, fen))
		pos = defaultPosition();
	movestack.reserve(MOVE_STACK);
	history.reserve(HIST_STACK);
}

void Game::load(std::string fen)
{
	Position p;
	if (getPositionFromFEN(p, fen))
	{
		pos = p;
		movestack.clear();
		history.clear();
	}
}

Move Game::getMove(int m) const
{
	Move move, imove;
	imove.x = m;
	std::set<int> moves = genMoves();
	
	if (imove.m.mtype & 32)
	{
		for (int x: moves)
		{
			move.x = x;
			if (move.m.mtype & 32 && move.m.detail == imove.m.detail)
				return move;
		}
	}

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

bool Game::makeMove(Move m)
{
	if (m.m.mtype & 128)
		return false;
	
	History h;
	h.m = m;
	h.capture = pos.squares[m.m.to].ptype;
	h.castleRights = pos.castleRights;
	h.ep = pos.enpassant;
	h.fifty = pos.fifty;
	h.hash = pos.hash;
	history.push_back(h);

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
	++pos.ply;

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
	if (history.empty())
		return false;
	History h = history[history.size() - 1];
	Move m;

	std::swap(pos.side, pos.xside);
	--pos.ply;
	history.pop_back();
	m = h.m;
	pos.castleRights = h.castleRights;
	pos.enpassant = h.ep;
	pos.fifty = h.fifty;
	pos.hash = h.hash;
	pos.squares[m.m.from].color = pos.side;
	
	if (m.m.mtype & 16) // Pawn promotion so change back to pawn.
		pos.squares[m.m.from].ptype = pawn;
	else
		pos.squares[m.m.from].ptype = pos.squares[m.m.to].ptype;
	
	if (h.capture == any) // Restore captured piece.
		pos.squares[m.m.to] = {none, any};
	else
		pos.squares[m.m.to] = {pos.xside, h.capture};
	
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

bool Game::inCheck(Color c) const
{
	for (int i = 0; i < NSQUARES; ++i)
	{
		if (pos.squares[i].color == c && pos.squares[i].ptype == king)
			return isAttacked(i, (c == white) ? black : white);
	}
	// Should not reach here, because that means there is no king
	return true;
}

std::set<int> Game::genMoves() const
{
	std::set<int> moves;
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
						generatePawnPromotionMoves(i, singlePawnMove, promotionMoves);
						for (int i = 0; i < 4; ++i)
                        	moves.insert(promotionMoves[i]);
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
						generatePawnPromotionMoves(i, n, promotionMoves);
						for (int i = 0; i < 4; ++i)
							moves.insert(promotionMoves[i]);
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
				// pawn capture square
				n = mailbox[mailbox64[i] + m * -11];
				if (n != -1 && pos.squares[n].color == pos.xside)
				{
                    if(pawnPromotionWhite || pawnPromotionBlack)
                    {
						generatePawnPromotionMoves(i, n, promotionMoves);
                        for (int i = 0; i < 4; ++i)
							moves.insert(promotionMoves[i]);
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
		moves.insert(move.x);
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
		moves.insert(move.x);
	}
	return moves;
}

std::vector<int> Game::genLegalMoves()
{
	std::vector<int> legalmoves;
	std::set<int> moves = genMoves();
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

	std::set<int> moves = genMoves();
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
