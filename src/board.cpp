#include "board.h"


#include <iostream>
void Board::init()
{
	bitBoards.push_back(BitBoard({white, pawn}, 0x00ff000000000000));
	bitBoards.push_back(BitBoard({white, knight}, 0x4200000000000000));
	bitBoards.push_back(BitBoard({white, bishop}, 0x2400000000000000));
	bitBoards.push_back(BitBoard({white, rook}, 0x8100000000000000));
	bitBoards.push_back(BitBoard({white, queen}, 0x1000000000000000));
	bitBoards.push_back(BitBoard({white, king}, 0x0800000000000000));
	bitBoards.push_back(BitBoard({black, pawn}, 0x000000000000ff00));
	bitBoards.push_back(BitBoard({black, knight}, 0x0000000000000042));
	bitBoards.push_back(BitBoard({black, bishop}, 0x0000000000000024));
	bitBoards.push_back(BitBoard({black, rook}, 0x0000000000000081));
	bitBoards.push_back(BitBoard({black, queen}, 0x0000000000000010));
	bitBoards.push_back(BitBoard({black, king}, 0x0000000000000008));

	Piece p;
	squares.resize(64);
	for (int i = 0; i < 64; ++i)
	{
		p = {none, any};
		for (const BitBoard & bb: bitBoards)
		{
			if (bb.isOccupied(i))
			{
				p = bb.piece;
				break;
			}
		}
		squares[i] = p;
	}

	side = white;
	xside = black;
	hply = 0;
	castleRights = 15;
	fifty = 0;
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
		enpassant = 0;

		int rshift = (side == white) ? 0: 56;
		uint8_t b = (side == white) ? 4: 1;

		if (m.m.mtype & 32)
		{
			// castling
			if (m.m.detail & 1)
			{
				squares[A1 + rshift] = {none, any};
				squares[E1 + rshift] = {none, any};
				squares[C1 + rshift] = {side, king};
				squares[D1 + rshift] = {side, rook};
			}
			else if (m.m.detail & 2)
			{
				squares[E1 + rshift] = {none, any};
				squares[H1 + rshift] = {none, any};
				squares[F1 + rshift] = {side, rook};
				squares[G1 + rshift] = {side, king};
			}

			castleRights &= ~(b << 1);
			castleRights &= ~b;
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
			squares[m.m.to] = {side, promotionPiece};
			squares[m.m.from] = {none, any};
			
            
		}
		else if(m.m.mtype & 8)
		{
			// double pawn move
			squares[m.m.to] = squares[m.m.from];
			squares[m.m.from] = {none, any};
			enpassant = (m.m.to + m.m.from) / 2;
		}
		else if(m.m.mtype & 2)
		{
			// enpassant capture
			squares[m.m.to] = squares[m.m.from];
			squares[m.m.from] = {none, any};
			if (side == white)
				squares[m.m.to - 8] = {none, any};
			else
				squares[m.m.to + 8] = {none, any};
		}
		else
		{
			// If moving king or rook from their starting squares, lose castling rights.
			if (m.m.from == (A1 + rshift) && squares[m.m.from].ptype == rook)
				castleRights &= ~(b << 1);
			else if (m.m.from == (H1 + rshift) && squares[m.m.from].ptype == rook)
				castleRights &= ~b;
			else if (m.m.from == (E1 + rshift) && squares[m.m.from].ptype == king)
			{
				castleRights &= ~(b << 1);
				castleRights &= ~b;
			}
			squares[m.m.to] = squares[m.m.from];
			squares[m.m.from] = {none, any};
		}
		
		if (m.m.mtype)
			fifty = 0;
		else
			++fifty;
		swap(side, xside);
		++hply;
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
				if (squares[n].color != none)
				{
					if (squares[n].color == c && (squares[n].ptype - pawn) == i)
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
	if (n != -1 && squares[n].color == c && squares[n].ptype == pawn)
		return true;
	n = mailbox[mailbox64[square] + m * 11];
	if (n != -1 && squares[n].color == c && squares[n].ptype == pawn)
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
		if (squares[i].color == side)
		{
			p = squares[i];
			if (p.ptype != pawn)
			{
				for (int j = 0; j < offsets[p.ptype - pawn]; ++j)
				{
					for (int n = i;;)
					{
						n = mailbox[mailbox64[n] + offset[p.ptype - pawn][j]];
						if (n == -1) break;
						if (squares[n].color != none)
						{
							if (squares[n].color == xside)
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
				int n, m = (side == white) ? 1 : -1;
				uint8_t singlePawnMove = static_cast<uint8_t>(i + m * 8);
				uint8_t doublePawnMove = static_cast<uint8_t>(i + m * 16);
				bool pawnPromotionWhite = side == white && (i / 8 == 6 );
				bool pawnPromotionBlack = side == black && (i / 8 == 1 );
				if (squares[singlePawnMove].color == none)
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
					if (side == white && (i / 8 == 1) && squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.insert(move.x);
					}
                    //double pawn for black
					if (side == black && (i / 8 == 6) && squares[doublePawnMove].color == none)
					{
						move.m = {8, i, doublePawnMove, 0};
						moves.insert(move.x);
					}

				}
				//pawn capture square
				n = mailbox[mailbox64[i] + m * -9];
				if (n != -1 && squares[n].color == xside)
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
				if (n == enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.insert(move.x);
				}
				//pawn capture square
				n = mailbox[mailbox64[i] + m * -11];
				if (n != -1 && squares[n].color == xside)
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
				if (n == enpassant)
				{
					move.m = {2, i, static_cast<uint8_t>(n), 0};
					moves.insert(move.x);
				}
			}

		}
	}

	// castling moves
	int rshift = (side == white) ? 0: 56;
	uint8_t b = (side == white) ? 4: 1;
	if ((castleRights & (b << 1)) &&
		(squares[B1 + rshift].color == none) &&
		(squares[C1 + rshift].color == none) &&
		(squares[D1 + rshift].color == none) &&
		!isAttacked(C1 + rshift, xside) &&
		!isAttacked(D1 + rshift, xside) &&
		!isAttacked(E1 + rshift, xside))
	{
		move.m = {32, 0, 0, 1};
		moves.insert(move.x);
	}
	if ((castleRights & b) &&
		(squares[F1 + rshift].color == none) &&
		(squares[G1 + rshift].color == none) &&
		!isAttacked(E1 + rshift, xside) &&
		!isAttacked(F1 + rshift, xside) &&
		!isAttacked(G1 + rshift, xside))
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



// Returns -1 if not a valid piece
	int Board::getPieceIndex(Piece p)
{
	if ((p.color == white || p.color == black) && p.ptype >= pawn && p.ptype <= king)
		return (p.color - white) * 6 + (p.ptype - pawn);
	return -1;
}

	char Board::getPieceLetter(Piece p)
{
	char pieceLetters[13] = "PNBRQKpnbrqk";
	int i = getPieceIndex(p);
	if (i == -1)
		return '.';
	return pieceLetters[i];
}

/*
	Same display board function as TSCP.
	TSCP was the first chess engine I examined so as I develop my own
	engine, I'm sure I will get a lot of help and inspiration from TSCP.

	http://www.tckerrigan.com/Chess/TSCP/
*/
void Board::display(ostream & os) const
{
	char square;
	int i;
	os << '\n';
	for (int rank = 7; rank >= 0; --rank)
	{
		os << rank + 1 << "  ";
		for (int file = 0; file < 8; ++file)
		{
			i = rank * 8 + file;
			square = getPieceLetter(squares[i]);
			os << square << ' ';
		}
		os << '\n';
	}
	os << "\n   a b c d e f g h\n\n";
}
