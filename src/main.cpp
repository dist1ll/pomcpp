#include <iostream>
#include <chrono>
#include <thread>

#include "bboard.hpp"
#include "agents.hpp"

int main()
{
    // Init
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    bboard::State* init = sx.get();
    bboard::InitState(init, 0, 1, 2, 3);

    init->PutItem(1, 4, bboard::Item::INCRRANGE);
    init->PutItem(6, 4, bboard::Item::KICK);
    init->PutItem(7, 6, bboard::Item::EXTRABOMB);

    agents::RandomAgent r;
    bboard::Agent* agents[4] = {&r, &r, &r, &r};
    bboard::StartGame(init, agents, 500);
}


