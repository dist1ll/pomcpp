#include "bboard.hpp"
#include "strategy.hpp"
#include "step_utility.hpp"

namespace bboard::strategy
{

////////////////////
// RMap Functions //
////////////////////

inline void ClearReachable(RMap& r, int x, int y)
{
    int* i = r.map + (x + BOARD_SIZE * y)/32;  // use correct integer
    *i &= ~(1 << ((x + BOARD_SIZE * y) % 32)); // set correct position
}

inline void SetReachable(RMap& r, int x, int y)
{
    int* i = r.map + (x + BOARD_SIZE * y)/32; // use correct integer
    *i |= 1 << ((x + BOARD_SIZE * y) % 32);   // set correct position
}

inline RMapInfo InspectPositionRecursively(State& s, RMap& r, int x, int y)
{
    if(!util::IsOutOfBounds(x, y)
            && s.board[y][x] == Item::PASSAGE
            && !IsReachable(r, x, y))
    {
        SetReachable(r, x, y);
        return FillRMap(s, r, x, y);
    }
    return 0;
}

RMapInfo FillRMap(State& s, RMap& r, int x, int y)
{
    InspectPositionRecursively(s, r, x, y + 1);
    InspectPositionRecursively(s, r, x, y - 1);
    InspectPositionRecursively(s, r, x + 1, y);
    InspectPositionRecursively(s, r, x - 1, y);
    return 0;
}

///////////////////////
// General Functions //
///////////////////////

void PrintMap(RMap& r)
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            std::cout << IsReachable(r, j, i) << " ";
        }
        std::cout << std::endl;
    }
}

// TODO ADD TEAM SUPPORT
bool IsAdjacentEnemy(const State& state, int agentID, int distance)
{
    const AgentInfo& a = state.agents[agentID];

    for(int i = 0; i < bboard::AGENT_COUNT; i++)
    {
        if(i == agentID) continue;

        // manhattan dist
        if((std::abs(state.agents[i].x - a.x) +
                std::abs(state.agents[i].y - a.y)) <= distance)
        {
            return true;
        }
    }
    return false;
}

bool CanSafelyBomb(const State& state, int agentID)
{
    return true;
}

}
