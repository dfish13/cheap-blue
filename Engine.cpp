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
    moveStack.reserve(MOVE_STACK);
    maxDepth = 8;
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
    firstMove[0] = 0;
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
                    (*os) << ' ' << getMoveString(pv[0][j]);
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
        moveStack.clear();
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

    firstMove[ply + 1] = game->genMoves(moveStack) + firstMove[ply];

    f = false;
    for (i = firstMove[ply]; i < firstMove[ply + 1]; ++i)
    {
        if (!game->makeMove(moveStack[i]))
            continue;

        f = true;
        ++ply;
        x = -search(-beta, -alpha, depth - 1);
        game->takeBack();
        --ply;

        if (x > alpha) // Cutoff
        {       
            if (x >= beta)
            {
                moveStack.erase(moveStack.begin() + firstMove[ply], moveStack.end());
                return beta;
            }
            alpha = x;
            pv[ply][ply] = moveStack[i];
            for (j = ply + 1; j < pvLength[ply + 1]; ++j)
				pv[ply][j] = pv[ply + 1][j];
			pvLength[ply] = pvLength[ply + 1];
        }
    }

    moveStack.erase(moveStack.begin() + firstMove[ply], moveStack.end());
    
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

    firstMove[ply + 1] = game->genCaptures(moveStack) + firstMove[ply];

    for (i = firstMove[ply]; i < firstMove[ply + 1]; ++i)
    {
        if (!game->makeMove(moveStack[i]))
            continue;
        ++ply;
        x = -quiesce(-beta, -alpha);
        game->takeBack();
        --ply;

        if (x > alpha) // Cutoff
        {       
            if (x >= beta)
            {
                moveStack.erase(moveStack.begin() + firstMove[ply], moveStack.end());
                return beta;
            }
            alpha = x;
            pv[ply][ply] = moveStack[i];
            for (j = ply + 1; j < pvLength[ply + 1]; ++j)
				pv[ply][j] = pv[ply + 1][j];
			pvLength[ply] = pvLength[ply + 1];
        }
    }
    moveStack.erase(moveStack.begin() + firstMove[ply], moveStack.end());

    return alpha;
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