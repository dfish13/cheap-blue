#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <atomic>
#include <climits>
#include <thread>
#include <chrono>
#include <format> // For std::format (C++20)

#include "Game.h"
#include "Engine.h"
#include "Util.h"

using namespace std;

void uci_loop();
void handle_position(istringstream &iss, Game &game);
void handle_go(istringstream &iss, Engine &engine, Game &game);
void go(Game g, atomic<bool> * stop);

atomic<bool> stop = false;
atomic<bool> debug = false;
ofstream fout;

int main(int argc, char **argv)
{
    if (argc > 1 && std::strcmp(argv[1], "-f"))
        debug = true;

    uci_loop();
    return 0;
}

void open_log_file()
{
    // Example of the very popular RFC 3339 format UTC time
    std::time_t time = std::time({});
    char timeString[std::size("hh-mm-ss")];
    std::strftime(std::data(timeString), std::size(timeString),
                  "%H-%M-%S", std::localtime(&time));
    string filename = "uci-";
    string timeS(timeString);
    fout.open(filename + timeS + ".log");
}

void uci_loop()
{
    string line, token;
    Game game;
    game.init();
    Engine engine({true, true, 8}, &stop);

    while (getline(cin, line))
    {
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
        else if (token == "debug")
        {
            debug = !debug;
            if (debug)
                engine.debug(&std::cout);
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
            move.x = parseMove(token, true);
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
            std::thread t(go, game, &stop);
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
        fout << "search_time = " << search_time <<'\n';
    }

    engine.think(game, search_time);
    Move best_move = engine.move();

    if (best_move.m.mtype & 128)
    {
        cout << "bestmove (none)" << endl;
    }
    else
    {
        cout << "bestmove " << getMoveStringRaw(best_move, true) << endl;
    }
}

void go(Game game, atomic<bool> * stop)
{
    Engine engine({true, true, 8}, stop);
    if (debug)
        engine.debug(&std::cout);
    engine.think(game, INT32_MAX);
    Move best_move = engine.move();

    if (best_move.m.mtype & 128)
    {
        cout << "bestmove (none)" << endl;
    }
    else
    {
        cout << "bestmove " << getMoveStringRaw(best_move, true) << endl;
    }
}
