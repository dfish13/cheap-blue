#include <iostream>
#include <vector>
#include <set>
#include <utility>

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

struct Move
{
	Move(int f, int t, bool c): from(f), to(t), capture(c) {}

	bool operator<(const Move &m) const
	{
		if (from == m.from)
			return to < m.to;
		return from < m.from;
	}

	int from, to;
	bool capture;
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

	bool checkMove(int from, int to) const
	{
		set<Move> moves = genMoves();
		Move m = {from, to, false}, mcap = {from, to, true};
		if (moves.find(m) != moves.end() || moves.find(mcap) != moves.end())
			return true;
		return false;
	}

	void makeMove(Move m)
	{
		squares[m.to] = squares[m.from];
		squares[m.from] = {none, any};
		swap(side, xside);
		++hply;
	}

	bool makeMove(int from, int to)
	{
		if (checkMove(from, to))
		{
			squares[to] = squares[from];
			squares[from] = {none, any};
			swap(side, xside);
			++hply;
			return true;
		}
		return false;
	}

	set<Move> genMoves() const
	{
		set<Move> moves;

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
		for (int i = 0; i < 64; ++i)
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
									moves.insert({i, n, true});
								break;
							}
							moves.insert({i, n, false});
							if (!slide[p.ptype - pawn]) break;
						}
					}
				}
				else // pawn moves
				{
					int n, m = (side == white) ? 1 : -1;
					if (squares[i + m * 8].color == none)
					{
						moves.insert({i, i + m * 8, false});
						if (side == white && (i / 8 == 1) && squares[i + m * 16].color == none)
							moves.insert({i, i + m * 16, false});
						if (side == black && (i / 8 == 6) && squares[i + m * 16].color == none)
							moves.insert({i, i + m * 16, false});
					}
					n = mailbox[mailbox64[i] + m * -9];
					if (n != -1 && squares[n].color == xside)
						moves.insert({i, n, true});
					n = mailbox[mailbox64[i] + m * -11];
					if (n != -1 && squares[n].color == xside)
						moves.insert({i, n, true});	
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
	bool qcastle, kcastle;
};


/*
	A valid move string is something of the form e2e4, b1c3, etc.
	Does not check if the move specified is legal.
	If invalid move string, returns a pair where first value is -1.

	Returns the indices of the square to move from and to 0-63.
*/
pair<int, int> parseMove(string m)
{
	if (m.size() == 4
	 && (m[0] >= 97 && m[0] < 105)	// a-h
	 && (m[1] >= 49 && m[1] < 57)	// 1-8
	 && (m[2] >= 97 && m[2] < 105)
	 && (m[3] >= 49 && m[3] < 57)
	)
		return make_pair(8 * (m[1] - 49) + (m[0] - 97), 8 * (m[3] - 49) + (m[2] - 97));
	else
		return make_pair(-1, 0);
}



int main()
{

	pair<int, int> move;
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
			move = parseMove(s);
			if (s == "q")
			{
				cout << "Bye!\n";
				exit(0);
			}
			else if (move.first == -1)
			{
				cout << "Invalid move string. Try again: ";
				continue;
			}	
			else if (!board.makeMove(move.first, move.second))
			{
				cout << "Illegal move. Try again: ";
				continue;
			}
			break;	
		}
	}
	
	return 0;
}
