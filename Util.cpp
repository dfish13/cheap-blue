#include "Util.h"
#include "Defs.h"
#include "BitBoard.h"

// Converts an index which implicitly uses the top left as the starting point
// to one that uses the bottom left.
// For example in top left indexing the square A8 corresponds to index 0
// and in bottom left A8 would be 56.
// C6 is 18 in top left indexing and 42 in bottom left indexing.
#define TOPLEFTTOBOTTOMLEFT(x) ((7 - ROW(x)) * 8 + COL(x)) 

int parseSquare(std::string s)
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
int parseMove(std::string m)
{
	Move move;

	uint8_t mtype = 0, from, to, detail;
	int promotion = 0;
	char promoteChars[5] = "qrbn";
	if (m == "O-O")
		move.m = {32, 0, 0, 2};
	else if (m == "O-O-O")
		move.m = {32, 0, 0, 1};
	else if (m.size() >= 4)
	{
		from = parseSquare(m.substr(0, 2));
		to = parseSquare(m.substr(2, 2));
		if (m.size() == 6)
		{
			for (int i = 0; i < 4; ++i)
				if (m[5] == promoteChars[i])
				{
					promotion = 1 << (3 - i);
					break;
				}
		}

		if (promotion){
			mtype |= 16;
		}
		detail |= promotion;
		
		if (from >= 0 && to >= 0)
			move.m = {mtype, from, to, detail};
		else 
			move.m = {128, 0, 0, 0};
	}
	else
		move.m = {128, 0, 0, 0};

	return move.x;
}

std::string indexToSquare(int i)
{
	if ( i < 0 || i >= NSQUARES)
		return "-";
	char s[3] = "a1";
	s[0] = (i % 8) + 'a';
	s[1] = (i / 8) + '1';

	return s;
}

std::string getMoveString(Move m)
{

	if (m.m.mtype & 32)
		return (m.m.detail & 2) ? "O-O" : "O-O-O";

	std::string s = indexToSquare(m.m.from) + indexToSquare(m.m.to);

	if (m.m.mtype & 128)
		return "None";
	
	if (m.m.mtype & 16)
	{
		s.push_back('=');
		if (m.m.detail & 8)
			s.push_back('q');
		else if (m.m.detail & 4)
			s.push_back('r');
		else if (m.m.detail & 2)
			s.push_back('b');
		else if (m.m.detail & 1)
			s.push_back('n');
	}
	return s;
}

// Used for converting chars to Piece objects and vice versa
char pieceLetters[14] = "PNBRQKpnbrqk.";

char getCharacterFromPiece(Piece p)
{
	int pieceIndex = 12;
	if ((p.color == white || p.color == black) && p.ptype >= pawn && p.ptype <= king)
		pieceIndex = (p.color - white) * 6 + (p.ptype - pawn);
	return pieceLetters[pieceIndex];
}

Piece getPieceFromCharacter(char c)
{
	Piece p = {none, any};
	int pieceIndex = -1;
	for (int i = 0; i < 12; ++i)
	 	if (c == pieceLetters[i])
			pieceIndex = i;
	
	if (pieceIndex == -1)
		return p;
	
	p.color = (pieceIndex < 6) ? white : black;
	
	switch (pieceIndex % 6)
	{
		case 0:
			p.ptype = pawn; break;
		case 1:
			p.ptype = knight; break;
		case 2:
			p.ptype = bishop; break;
		case 3:
			p.ptype = rook; break;
		case 4:
			p.ptype = queen; break;
		case 5:
			p.ptype = king; break;
	}

	return p;
}

Position defaultPosition()
{
	std::list<BitBoard> bitBoards;

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

	Position pos;
	Piece p;
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
		pos.squares[i] = p;
	}

	pos.side = white;
	pos.xside = black;
	pos.castleRights = 15;
	pos.fifty = 0;
	pos.enpassant = NSQUARES;

	return pos;
}

bool getPositionFromFEN(Position & pos, std::string fen)
{
	Piece p;
	size_t i, toplefti = 0;

	for (i = 0; i < NSQUARES; ++i)
		pos.squares[i] = {none, any};
	
	for (i = 0; i < fen.size() && fen[i] != ' '; ++i)
	{
		if (isdigit(fen[i]))
			toplefti += (fen[i] - '0');
		else if (fen[i] == '/')
			continue;
		else
		{
			pos.squares[TOPLEFTTOBOTTOMLEFT(toplefti)] = getPieceFromCharacter(fen[i]);
			++toplefti;
		}
	}

	if (fen[++i] == 'w')
		pos.side = white;
	else if (fen[i] == 'b')
		pos.side = black;
	else
		return false;
	pos.xside = (pos.side == white) ? black : white;
	pos.castleRights = 0;
	
	for (i += 2; i < fen.size() && fen[i] != ' '; ++i)
	{
		switch (fen[i])
		{
			case 'Q':
				pos.castleRights |= 8; break;
			case 'K':
				pos.castleRights |= 4; break;
			case 'q':
				pos.castleRights |= 2; break;
			case 'k':
				pos.castleRights |= 1; break;
			default:
				break;
		}
	}

	if (fen[++i] != '-')
	{
		pos.enpassant = parseSquare(fen.substr(i, 2));
		if (pos.enpassant < 0)
			return false;
	}
	else
		pos.enpassant = NSQUARES;

	i += 2;
	if (i < fen.size())
	{
		std::istringstream iss(fen.substr(i));
		iss >> pos.fifty;
		// We dont care about full move counter.
	}
	else
		pos.fifty = 0;

	return true;
}

void printBoard(std::ostream & os, const Position & p)
{
	int i;
	os << '\n';
	for (int rank = 7; rank >= 0; --rank)
	{
		os << rank + 1 << "  ";
		for (int file = 0; file < 8; ++file)
		{
			i = rank * 8 + file;
			os << getCharacterFromPiece(p.squares[i]) << ' ';
		}
		os << '\n';
	}
	os << "\n   a b c d e f g h\n\n";
}

void printPosition(std::ostream & os, const Position & p)
{
	std::string s;

	printBoard(os, p);
	if (p.side == white)
		s = "white\n";
	else if (p.side == black)
		s = "black\n";
	else
		s = "invalid\n";
	os << "side = " << s;
	os << "enpassant = " << indexToSquare(p.enpassant) << '\n';
	os << "fifty = " << p.fifty << '\n';

	s.clear();
	if (p.castleRights & 8)
		s.push_back('Q');
	if (p.castleRights & 4)
		s.push_back('K');
	if (p.castleRights & 2)
		s.push_back('q');
	if (p.castleRights & 1)
		s.push_back('k'); 
	
	os << "castleRights = " << ((s.empty()) ? "-" : s) << '\n';
}

bool checkPosition(const Position & p, std::ostream & os)
{
    for (int i = 0; i < NSQUARES; ++i)
    {
		if (p.squares[i].color == white && i >= 48)
		{
			os << i << '\n';
			return false;
		}
			
		if (p.squares[i].color == black && i < 16)
		{
			os << i << '\n';
			return false;
		}
        if (p.squares[i].ptype < pawn || p.squares[i].ptype > any)
            return false;
        if (p.squares[i].color < white || p.squares[i].color > none)
            return false;
    }
	return true;
}