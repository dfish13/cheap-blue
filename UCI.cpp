#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <atomic>
#include <climits>
#include <thread>

#include "Game.h"
#include "Engine.h"
#include "Util.h"

using namespace std;

void uci_loop();
void handle_position(istringstream &iss, Game &game);
void handle_go(istringstream &iss, Engine &engine, Game &game);
void go(Engine engine, atomic<bool> * stop);

atomic<bool> stop = false;

int main(int argc, char **argv)
{
    uci_loop();
    return 0;
}

void uci_loop()
{
    // Log incoming UCI messages.
    std::ofstream fout("uci_log.b");

    string line, token;
    Game game;
    game.init();
    Engine engine(&game, {true, true}, &stop);

    while (getline(cin, line))
    {
        fout << line << '\n';
        istringstream iss(line);
        iss >> token;

        if (token == "uci")
        {
            cout << "id name Cheap Blue" << endl;
            cout << "id author Duncan Fisher" << endl;
            cout << "uciok" << endl;
        }
        else if (token == "isready")
        {
            cout << "readyok" << endl;
        }
        else if (token == "ucinewgame")
        {
            game.init();
        }
        else if (token == "position")
        {
            handle_position(iss, game);
        }
        else if (token == "go")
        {
            handle_go(iss, engine, game);
        }
        else if (token == "stop")
        {
            stop = true;
        }
        else if (token == "quit")
        {
            break;
        }
    }
}

void handle_position(istringstream &iss, Game &game)
{
    string token;
    iss >> token;

    if (token == "startpos")
    {
        game.init();
        iss >> token; // consume "moves" if present
    }
    else if (token == "fen")
    {
        string fen = "";
        // Read FEN string (up to 6 parts)
        for (int i = 0; i < 6 && iss >> token; i++)
        {
            if (token == "moves")
                break;
            if (!fen.empty())
                fen += " ";
            fen += token;
        }
        game.init(fen);
    }

    // Apply moves if present
    if (token == "moves" || iss >> token)
    {
        if (token == "moves")
            iss >> token;

        do {
            Move move;
            move.x = parseMove(token);
            if (!(move.m.mtype & 128))
            {
                move = game.getMove(move.x);
                game.makeMove(move);
            }
        } while (iss >> token);
    }
}

void handle_go(istringstream &iss, Engine &engine, Game &game)
{
    string token;
    int wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 0;
    int movetime = 0, depth = 0;

    stop = false;

    while (iss >> token)
    {
        if (token == "wtime")
            iss >> wtime;
        else if (token == "btime")
            iss >> btime;
        else if (token == "winc")
            iss >> winc;
        else if (token == "binc")
            iss >> binc;
        else if (token == "movestogo")
            iss >> movestogo;
        else if (token == "movetime")
            iss >> movetime;
        else if (token == "depth")
            iss >> depth;
        else if (token == "infinite") {
            std::thread t(go, engine, &stop);
            t.detach();
            return;
        }
    }

    // Simple time management
    int search_time;
    if (movetime > 0)
    {
        search_time = movetime;
    }
    else
    {
        int time_left = (game.pos.side == white) ? wtime : btime;
        int increment = (game.pos.side == white) ? winc : binc;

        if (movestogo > 0)
            search_time = (time_left / movestogo) + increment;
        else
            search_time = (time_left / 30) + increment; // Assume 30 moves to go

        // Safety margins
        search_time = min(search_time, time_left / 2);
        search_time = max(search_time, 100); // At least 100ms
    }

    engine.think(search_time);
    Move best_move = engine.move();

    if (best_move.m.mtype & 128)
    {
        cout << "bestmove (none)" << endl;
    }
    else
    {
        cout << "bestmove " << getMoveStringRaw(best_move) << endl;
    }
}

void go(Engine engine, atomic<bool> * stop)
{
    engine.addStop(stop);
    engine.think(INT32_MAX);
    Move best_move = engine.move();

    if (best_move.m.mtype & 128)
    {
        cout << "bestmove (none)" << endl;
    }
    else
    {
        cout << "bestmove " << getMoveStringRaw(best_move) << endl;
    }
}
