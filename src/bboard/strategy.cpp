#include "bboard.hpp"
#include "strategy.hpp"

namespace bboard::strategy
{

// TODO ADD TEAM SUPPORT
bool IsAdjacentEnemy(const State& state, int agentID, int distance)
{
    const AgentInfo& a = state.agents[agentID];

    for(int i = 0; i < bboard::AGENT_COUNT; i++)
    {
        if(i == agentID) continue;

        // manhattan dist
        if((std::abs(state.agents[i].x - a.x) +
                std::abs(state.agents[i].y - a.y)) < distance)
        {
            return true;
        }
    }
    return false;
}

}
