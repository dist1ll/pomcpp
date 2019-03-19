#include <iostream>
#include <chrono>
#include <thread>

#include "bboard.hpp"
#include "agents.hpp"

int main()
{

    agents::SimpleAgent r[4];
    agents::SimpleAgent g[4];
    std::array<bboard::Agent*, 4> agents = {&r[0], &g[1], &g[2], &g[3]};

    bboard::Environment env;
    env.MakeGame(agents, true);

    for(bboard::AgentInfo& agentInfo : env.GetState().agents)
    {
        agentInfo.canKick = true;
    }

    env.StartGame(500, true);

}
