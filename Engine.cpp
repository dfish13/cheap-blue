#include "Engine.h"

bool globalFlag;

Engine::Engine(Game * g) : game(g), verbose(false)
{
    init();
}

Engine::Engine(Game * g, std::ostream * o) : game(g), os(o), verbose(true)
{
    init();
}

void Engine::init()
{
    maxDepth = 20;
}

void Engine::think(int ms)
{
    int i, j, x;
    globalFlag = true;
    std::chrono::duration<int, std::milli> duration(ms);
    end = std::chrono::high_resolution_clock::now() + duration; 

    ply = 0;
    nodes = 0;
    memset(pv, 0, sizeof(pv));
    try
    {
        if (verbose)
            (*os) << "ply      nodes  score  pv\n";
        for (i = 1; i <= maxDepth; ++i)
        {
            followPV = true;
            x = search(-10000, 10000, i);
            if (verbose)
            {
                (*os) << std::setw(3) << i;
                (*os) << std::setw(11) << nodes;
                (*os) << std::setw(7) << x;
                for (j = 0; j < pvLength[0]; ++j)
                    (*os) << "  " << getMoveString(pv[0][j]);
                (*os) << '\n';
            }
            if (x > 9000 || x < -9000)
			break;
        }
    }
    catch (TimeIsUp tiu)
    {
        if (verbose)
            (*os) << "Time is up!\n";
        while (ply--)
            game->takeBack();
    }
}

int Engine::search(int alpha, int beta, int depth)
{
    
    int i, j, x;
	bool c, f;
    if (depth == 0)
		return quiesce(alpha, beta);
	++nodes;
    
    if ((nodes & 1023) == 0)
        checkup();

    pvLength[ply] = ply;

    if (ply >= MAX_PLY - 1)
        return game->eval();

    c = game->inCheck(game->pos.side);
	if (c)
		++depth;
    std::vector<int> moves;
    moves.reserve(50);
    game->genMoves(moves);

    if (followPV)
        sortPV(moves);

    sort(moves);
    f = false;
    for (int m : moves)
    {
        if (!game->makeMove(m))
            continue;

        f = true;
        ++ply;
        x = -search(-beta, -alpha, depth - 1);
        game->takeBack();
        --ply;

        if (x > alpha) // Cutoff
        {       
            if (x >= beta)
                return beta;
            alpha = x;
            pv[ply][ply] = m;
            for (j = ply + 1; j < pvLength[ply + 1]; ++j)
				pv[ply][j] = pv[ply + 1][j];
			pvLength[ply] = pvLength[ply + 1];
        }
    }
    
    if (!f)
    {
        if (c)
            return -10000 + ply; // Checkmate
        else
            return 0; // Stalemate
    }

    if (game->pos.fifty >= 100) // 50 move draw
        return 0;
    return alpha;
}

int Engine::quiesce(int alpha, int beta)
{
    int i, j, x;
    ++nodes;

    if ((nodes & 1023) == 0)
		checkup();

    pvLength[ply] = ply;

    if (ply >= MAX_PLY - 1)
		return game->eval();

    x = game->eval();
	if (x >= beta)
		return beta;
	if (x > alpha)
		alpha = x;

    std::vector<int> moves;
    moves.reserve(25);
    game->genCaptures(moves);

    if (followPV)
        sortPV(moves);

    sort(moves);
    for (int m: moves)
    {
        if (!game->makeMove(m))
            continue;
        ++ply;
        x = -quiesce(-beta, -alpha);
        game->takeBack();
        --ply;

        if (x > alpha) // Cutoff
        {       
            if (x >= beta)
                return beta;
            alpha = x;
            pv[ply][ply] = m;
            for (j = ply + 1; j < pvLength[ply + 1]; ++j)
				pv[ply][j] = pv[ply + 1][j];
			pvLength[ply] = pvLength[ply + 1];
        }
    }

    return alpha;
}

void Engine::sortPV(std::vector<int> & moves)
{
	followPV = false;
	for(int i = 0; i < moves.size(); ++i)
		if (moves[i] == pv[0][ply])
        {
			followPV = true;
			std::swap(moves[0], moves[i]);
			return;
		}
}

void Engine::score(std::vector<int> & moves, int * scores)
{
    Move m;
    Piece * p = (game->pos.squares) ;
    for (int i = 0; i < moves.size(); ++i)
    {
        m.x = moves[i];
        if (m.m.mtype & 16)
            scores[i] = 1000000 + ((int) m.m.detail) * 10;
        else if (p[m.m.to].color != none)
            scores[i] = 1000000 + p[m.m.to].ptype * 10 - p[m.m.from].ptype;
        else
            scores[i] = 0;

    }
}

void Engine::sort(std::vector<int> & moves)
{
    int * scores = new int[moves.size()];
    score(moves, scores);

    int i, j;
    for (i = (followPV) ? 1 : 0; i < moves.size(); ++i)
        for (j = moves.size() - 1; j > i; --j)
            if (scores[j] > scores[j - 1])
            {
                std::swap(scores[j], scores[j - 1]);
                std::swap(moves[j], moves[j - 1]);
            }
    delete [] scores;
}

void Engine::checkup()
{
    now = std::chrono::high_resolution_clock::now();
    if (now >= end)
        throw TimeIsUp();
}

Move Engine::move()
{
    if (pv[0][0])
        return pv[0][0];
    Move m;
    m.m = {128, 0, 0, 0};
    return m;
}