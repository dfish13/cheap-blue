#include "Engine.h"

bool globalFlag;

Engine::Engine(EngineConfig ec) : verbose(false), stop(nullptr)
{
    init(ec);
}

Engine::Engine(std::ostream * o) : os(o), verbose(true), stop(nullptr)
{
    init();
}

Engine::Engine(EngineConfig ec, std::atomic<bool> * stop) : verbose(false), stop(stop)
{
    init(ec);
}

void Engine::init(EngineConfig ec)
{
    maxDepth = 20;
    config = ec;
    if (config.useBook)
        book.init();

    // Initialize transposition table.
    tt.init(ec.tt_size);

    // Initialize killer moves.
    memset(killerMoves, 0, sizeof(killerMoves));
}

void Engine::updateGame(Game g)
{
    this->game = g;
}

void Engine::debug(std::ostream * o)
{
    verbose = true;
    os = o;
}

void Engine::think(Game g, int ms)
{
    // Force a game update to ensure we aren't relying on some outdated game state.
    updateGame(g);

    if (config.useBook)
    {
        Move m;
        if (book.getMove(game.pos.hash, m))
        {
            pv[0][0] = m.x;
            if (verbose)
                (*os) << "Book move\n";
            return;
        }
        else
            config.useBook = false;
    }

    int i, j, x;
    std::chrono::duration<int, std::milli> duration(ms);
    end = std::chrono::high_resolution_clock::now() + duration; 
    tt_entry e;

    ply = 0;
    nodes = 0;
    memset(pv, 0, sizeof(pv));
    memset(killerMoves, 0, sizeof(killerMoves));
    try
    {
        if (verbose)
            (*os) << "ply      nodes  score  pv\n";
        for (i = 1; i <= maxDepth; ++i)
        {
            followPV = config.pvSort;
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
            game.takeBack();
    }
}

int Engine::search(int alpha, int beta, int depth)
{
    int i, j, x;
	bool c, f;

    // alpha gets updated as the score improves so we store the original alpha value.
    int alpha_old = alpha;
    tt_entry e;
    

    if (tt.probe(game.pos.hash, e) && e.depth >= depth)
    {
        if (e.flag == TT_EXACT)
        {
            bool has_three_fold = false;
            if (depth == 1)
            {
                game.makeMove(e.best_move);
                bool has_three_fold = game.hasThreefoldRepetition();
                game.takeBack();
            }
            if (!has_three_fold)
            {
                // Need to update pv table on exact TT hits.
                // TT_EXACT means alpha < score < beta which is the condition for which we update the pv in normal search.
                pv[ply][ply] = e.best_move;
                for (j = ply + 1; j < pvLength[ply + 1]; ++j)
                    pv[ply][j] = pv[ply + 1][j];
                pvLength[ply] = pvLength[ply + 1];
                return e.eval;
            }
        }
        if (e.flag == TT_ALPHA && e.eval <= alpha)
            return alpha;
        if (e.flag == TT_BETA && e.eval >= beta)
            return beta;
    } 
    
    if (game.pos.fifty >= 100) // 50 move draw
        return 0;

    if (game.hasThreefoldRepetition())
        return 0;

    if (depth == 0)
		return quiesce(alpha, beta);
    
    ++nodes;
    if ((nodes & 1023) == 0)
        checkup();

    pvLength[ply] = ply;

    if (ply >= MAX_PLY - 1)
        return game.eval();

    c = game.inCheck(game.pos.side);
	if (c)
		++depth;
    std::vector<int> moves;
    moves.reserve(50);
    game.genMoves(moves);

    if (followPV)
        sortPV(moves);

    sort(moves);
    f = false;
    int best_move = moves[0];
    for (int m : moves)
    {
        if (!game.makeMove(m))
            continue;

        f = true;
        ++ply;
        x = -search(-beta, -alpha, depth - 1);
        game.takeBack();
        --ply;

        if (x > alpha) // Cutoff
        {
            best_move = m;
            if (x >= beta)
            {
                // Store killer moves (only for quiet moves, not captures).
                Move move;
                move.x = m;
                if (!(move.m.mtype & 64)) // Not a capture
                {
                    // Shift killers: new killer goes to slot 0, old slot 0 goes to slot 1
                    if (killerMoves[ply][0] != m)
                    {
                        killerMoves[ply][1] = killerMoves[ply][0];
                        killerMoves[ply][0] = m;
                    }
                }
                tt.store(game.pos.hash, beta, TT_BETA, depth, best_move);
                return beta;
            }

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

    if (alpha == alpha_old)
        tt.store(game.pos.hash, alpha, TT_ALPHA, depth, best_move);
    else
        tt.store(game.pos.hash, alpha, TT_EXACT, depth, best_move);
    
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
		return game.eval();

    x = game.eval();
	if (x >= beta)
		return beta;
	if (x > alpha)
		alpha = x;

    std::vector<int> moves;
    moves.reserve(25);
    game.genCaptures(moves);

    if (followPV)
        sortPV(moves);

    sort(moves);
    for (int m: moves)
    {
        if (!game.makeMove(m))
            continue;
        ++ply;
        x = -quiesce(-beta, -alpha);
        game.takeBack();
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
    Piece * p = (game.pos.squares) ;
    for (int i = 0; i < moves.size(); ++i)
    {
        m.x = moves[i];
        if (m.m.mtype & 16) // Pawn promotion
        {
            if (m.m.mtype & 64) // Promotion and capture
                scores[i] = 2000000 + ((int) m.m.detail) * 10;
            else
                scores[i] = 1000000 + ((int) m.m.detail) * 10;
        }
        else if (m.m.mtype & 64) // Capture
            scores[i] = 1000000 + p[m.m.to].ptype * 10 - p[m.m.from].ptype;
        else if (moves[i] == killerMoves[ply][0]) // First killer move
            scores[i] = 900000;
        else if (moves[i] == killerMoves[ply][1]) // Second killer move
            scores[i] = 800000;
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

void Engine::addStop(std::atomic<bool> * stop)
{
    this->stop = stop;
}

void Engine::checkup()
{
    now = std::chrono::high_resolution_clock::now();
    if (now >= end)
        throw TimeIsUp();
    if (stop != nullptr && *stop)
        throw TimeIsUp();
}

Move Engine::move()
{
    Move m;
    if (pv[0][0])
        return pv[0][0];
    m.m = {128, 0, 0, 0};
    return m;
}