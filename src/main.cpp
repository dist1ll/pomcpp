#include <iostream>
#include <chrono>
#include <thread>

#include "bboard.hpp"
#include "agents.hpp"

using std::cout;

void Run(bboard::State* state)
{
    bboard::Move moves[4];
    agents::RandomAgent agent1;

    for(int i = 0; i < 500; i++)
    {
        cout << "\033c"; // clear console on linux

        moves[0] = agent1.act(state);
        moves[1] = agent1.act(state);
        moves[2] = agent1.act(state);
        moves[3] = agent1.act(state);

        bboard::Step(state, moves);
        bboard::PrintState(state);

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

int main()
{
    // Init
    bboard::State* init = bboard::InitState(0,1,2,3);
    Run(init);
    delete init;
}


