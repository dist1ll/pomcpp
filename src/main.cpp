#include <iostream>
#include <chrono>
#include <thread>

#include "bboard.hpp"
#include "agents.hpp"

int main()
{

    agents::SimpleAgent r[4];
    std::array<bboard::Agent*, 4> agents = {&r[0], &r[1], &r[2], &r[3]};

    bboard::Environment env;
    env.MakeGame(agents);
    env.StartGame(500, true);

}
