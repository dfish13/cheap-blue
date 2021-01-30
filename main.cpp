#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <cstdint>

enum Color {none, white, black, both};
enum PType {pawn, knight, bishop, rook, queen, king, any};

struct Piece
{
	Piece()
	{
		color = none;
		ptype = any;
	}

	Piece(Color c, PType p): color(c), ptype(p) {}
	Color color;
	PType ptype;
};

struct MoveBytes
{

	MoveBytes() {};
	MoveBytes(uint8_t i, uint8_t f, uint8_t t): info(i), from(f), to(t), pad(0) {}
	/*
		The info byte specifies what type of move it is.

		info & 128 = invalid
		info & 64 = capture
		info & 32 = castle
		info & 16 = promotion

		if the move is a castle,

		info & 2 = castle kingside
		info & 1 = castle queenside

		if the move is a promotion,

		info & 8 = queen
		info & 4 = rook
		info & 2 = bishop
		info & 1 = knight

	*/
	uint8_t info;
	uint8_t from;
	uint8_t to;
	uint8_t pad;
};

union Move
{
	Move() {};
	MoveBytes m;
	int x;
};

using namespace std;

int mailbox[120] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	-1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	-1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	-1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	-1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	-1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	-1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int mailbox64[64] = {
    91, 92, 93, 94, 95, 96, 97, 98,
	81, 82, 83, 84, 85, 86, 87, 88,
	71, 72, 73, 74, 75, 76, 77, 78,
	61, 62, 63, 64, 65, 66, 67, 68,
	51, 52, 53, 54, 55, 56, 57, 58,
	41, 42, 43, 44, 45, 46, 47, 48,
	31, 32, 33, 34, 35, 36, 37, 38,
	21, 22, 23, 24, 25, 26, 27, 28
};

class BitBoard
{
	public:

	static long mask(int i)
	{
		long m = 1;
		return m << (63 - i);
	}

	BitBoard(Piece p, long i) : piece(p), bb(i) {}

	bool isOccupied(int i) const
	{
		return mask(i) & bb;
	}

	Piece piece;
	long bb;
};

class Board
{
	public:

	Board() {}

	void init()
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

	bool checkMove(Move m) const
	{
		set<int> moves = genMoves();
		for (int m: moves)
		{
			Move move;
			move.x = m;
			cout << (int) move.m.from << ' ' << (int) move.m.to << '\n';
		}
		return moves.find(m.x) != moves.end();
	}

	bool makeMove(Move m)
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

	set<int> genMoves() const
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
	static int getPieceIndex(Piece p)
	{
		if ((p.color == white || p.color == black) && p.ptype >= pawn && p.ptype <= king)
			return (p.color - white) * 6 + (p.ptype - pawn);
		return -1;
	}

	static char getPieceLetter(Piece p)
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
	void display(ostream & os) const
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

	vector<BitBoard> bitBoards;
	vector<Piece> squares;
	Color side, xside;
	int hply;
	int enpassant;
	int castle;
};


int parseSquare(string s)
{
	if (s.size() == 2
		&& (s[0] >= 97 && s[0] < 105)	// a-h
		&& (s[1] >= 49 && s[1] < 57)	// 1-8
	)
		return 8 * (s[1] - 49) + (s[0] - 97);
	return -1;
}


/*
	A valid move string is something of the form e2e4, b1xc3, etc.
	Does not check if the move specified is legal.

	castle kingside = O-O
	castle Queenside = O-O-O

	For promotion add =<Piece> where <Piece> is in {q, r, b, n}
	e.g. h7h8=q or e7xf8=n
*/
int parseMove(string m)
{
	Move move;

	uint8_t info = 0, from, to;
	int capture = 0, promotion = 0;
	char promoteChars[5] = "qrbn";
	if (m == "O-O")
		move.m = {34, 0, 0};
	else if (m == "O-O-O")
		move.m = {33, 0, 0};
	else if (m.size() >= 4)
	{
		from = parseSquare(m.substr(0, 2));
		if (m[2] == 'x')
			capture = 1;
		to = parseSquare(m.substr(2 + capture, 2));
		if (m.size() == 6 + capture)
		{
			for (int i = 0; i < 4; ++i)
				if (m[5 + capture] == promoteChars[i])
				{
					promotion = 1 << (3 - i);
					break;
				}
		}

		if (capture)
			info |= 64;
		if (promotion)
			info |= 16;
		info |= promotion;
		if (from >= 0 && to >= 0)
			move.m = {info, from, to};
		else
			move.m = {128, 0, 0};
	}
	else
		move.m = {128, 0, 0};
	return move.x;
}



int main()
{

	Move move;
	string s;
	Board board;
	board.init();

	while (1)
	{
		board.display(cout);
		cout << "Enter move or q to quit: ";
		while (1)
		{
			cin >> s;
			move.x = parseMove(s);
			if (s == "q")
			{
				cout << "Bye!\n";
				exit(0);
			}
			else if (move.m.info & 128)
			{
				cout << "Invalid move string. Try again: ";
				continue;
			}
			else if (!board.makeMove(move))
			{
				cout << "Illegal move. Try again: ";
				continue;
			}
			break;
		}
	}

	return 0;
}
