#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <chrono>
#include <ostream>
#include <iomanip>
#include <cstring>
#include <atomic>

#include "Defs.h"
#include "Util.h"
#include "Game.h"
#include "Book.h"
#include "TT.h"

#define MAX_PLY		64

const char bookFile[] = "book";

struct TimeIsUp {};

class Engine
{

public:
    Engine(EngineConfig ec = {true /* pvSort on */, true /* book on */, 8 /* 8mb TT */});
    Engine(std::ostream * o);
    Engine(EngineConfig ec, std::atomic<bool> * stop);

    void init(EngineConfig ec  = {true, true});
    void updateGame(Game g);

    void think(Game g, int ms);
    int search(int alpha, int beta, int depth);
    int quiesce(int alpha, int beta);

    void sortPV(std::vector<int> & moves);
    void score(std::vector<int> & moves, int * scores);
    void sort(std::vector<int> & moves);

    void addStop(std::atomic<bool> * stop);
    void checkup();

    Move move();

    TT tt;
    EngineConfig config;

private:
    Game game;

    int pvLength[MAX_PLY];
    bool followPV;
    int maxDepth;
    int nodes;
    int ply;
    bool verbose;
    std::ostream * os; 
    std::chrono::high_resolution_clock::time_point now, end;
    
    // this boolean is controlled by the main thread.
    // the engine reads the value to decide if it should continue searching.
    // DO NOT write to this variable from member functions of Engine.
    std::atomic<bool>* stop;

    int pv[MAX_PLY][MAX_PLY];

    Book book;

};

#endif
