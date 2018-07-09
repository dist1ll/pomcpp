#include <iostream>
#include <chrono>
#include <thread>

#include "bboard.hpp"
#include "agents.hpp"

int main()
{

    agents::RandomAgent r;
    std::array<bboard::Agent*, 4> agents = {&r, &r, &r, &r};

    bboard::Environment env;
    env.MakeGame(agents);
    env.StartGame(500, true);

}


