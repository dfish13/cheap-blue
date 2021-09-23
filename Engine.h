#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <chrono>
#include <ostream>
#include <iomanip>
#include <cstring>

#include "Defs.h"
#include "Util.h"
#include "Game.h"

#define MAX_PLY		64

struct TimeIsUp {};

class Engine
{

public:
    Engine(Game * g);
    Engine(Game * g, std::ostream * o);

    void init();

    void think(int ms);
    int search(int alpha, int beta, int depth);
    int quiesce(int alpha, int beta);

    void sortPV(std::vector<int> & moves);
    void score(std::vector<int> & moves, int * scores);
    void sort(std::vector<int> & moves);

    void checkup();

    Move move();

private:
    Game * game;

    int pvLength[MAX_PLY];
    bool followPV;
    int maxDepth;
    int nodes;
    int ply;
    bool verbose;
    std::ostream * os; 
    std::chrono::high_resolution_clock::time_point now, end;

    int pv[MAX_PLY][MAX_PLY];

};

#endif
