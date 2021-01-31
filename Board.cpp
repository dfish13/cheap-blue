#include "Board.h"


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
	}

	bool Board::checkMove(Move m) const
	{
		set<int> moves = genMoves();
		for (int m: moves)
		{
			Move move;
			move.x = m;
		}
		return moves.find(m.x) != moves.end();
	}

	bool Board::makeMove(Move m)
	{
		if (checkMove(m))
		{
			squares[m.m.to] = squares[m.m.from];
			squares[m.m.from] = {none, any};
			swap(side, xside);
			++hply;
			return true;
		}
		return false;
	}

	set<int> Board::genMoves() const
	{
		set<int> moves;
		Move move;

		bool slide[6] = {false, false, true, true, true, false};
		int offsets[6] = {0, 8, 4, 4, 8, 8};
		int offset[6][8] = {
			{   0,   0,  0,  0, 0,  0,  0,  0 },
			{ -21, -19,-12, -8, 8, 12, 19, 21 },
			{ -11,  -9,  9, 11, 0,  0,  0,  0 },
			{ -10,  -1,  1, 10, 0,  0,  0,  0 },
			{ -11, -10, -9, -1, 1,  9, 10, 11 },
			{ -11, -10, -9, -1, 1,  9, 10, 11 }
		};

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
									move.m = {64, i, static_cast<uint8_t>(n)};
									moves.insert(move.x);
								}
								break;
							}
							move.m = {0, i, static_cast<uint8_t>(n)};
							moves.insert(move.x);
							if (!slide[p.ptype - pawn]) break;
						}
					}
				}
				else // pawn moves
				{
					int n, m = (side == white) ? 1 : -1;
					if (squares[i + m * 8].color == none)
					{
						move.m = {0, i, static_cast<uint8_t>(i + m * 8)};
						moves.insert(move.x);
						if (side == white && (i / 8 == 1) && squares[i + m * 16].color == none)
						{
							move.m = {0, i, static_cast<uint8_t>(i + m * 16)};
							moves.insert(move.x);
						}
						if (side == black && (i / 8 == 6) && squares[i + m * 16].color == none)
						{
							move.m = {0, i, static_cast<uint8_t>(i + m * 16)};
							moves.insert(move.x);
						}
					}
					n = mailbox[mailbox64[i] + m * -9];
					if (n != -1 && squares[n].color == xside)
					{
						move.m = {64, i, static_cast<uint8_t>(n)};
						moves.insert(move.x);
					}
					n = mailbox[mailbox64[i] + m * -11];
					if (n != -1 && squares[n].color == xside)
					{
						move.m = {64, i, static_cast<uint8_t>(n)};
						moves.insert(move.x);
					}
				}

			}
		}

		return moves;
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
