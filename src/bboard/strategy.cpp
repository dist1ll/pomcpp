#include "bboard.hpp"
#include "strategy.hpp"
#include "step_utility.hpp"

namespace bboard::strategy
{

////////////////////
// RMap Functions //
////////////////////

void RMap::SetDistance(int x, int y, int distance)
{
    map[y][x] = (map[y][x] & ~chalf) + distance;
}

void RMap::SetPredecessor(int x, int y, int xp, int yp)
{
    map[y][x] = (map[y][x] & chalf) + ((xp + BOARD_SIZE * yp) << 16);
}

int RMap::GetDistance(int x, int y)
{
    return map[y][x] & chalf;
}

int RMap::GetPredecessor(int x, int y)
{
    return map[y][x] >> 16;
}

template <typename T, int N>
inline void TryAddToQueue(State& s, FixedQueue<T, N>& queue, RMap& r, int dist, int cx, int cy)
{
    if(!util::IsOutOfBounds(cx, cy) &&
            r.GetDistance(cx, cy) == 0 &&
            s.board[cy][cx] != Item::RIGID)
    {
        r.SetDistance(cx, cy, dist + 1);
        queue.AddElem({cx, cy});
    }
}
// optimized dijkstra. See explanation optimization III in docs
RMapInfo FillRMap(State& s, RMap& r, int x, int y, int distance)
{
    // holds 1-d indices
    FixedQueue<Position, BOARD_SIZE * BOARD_SIZE> queue;
    r.SetDistance(x, y, 0);
    queue.AddElem({x, y});

    while(queue.count != 0)
    {

        Position& c = queue.PopElem();
        int dist = r.GetDistance(c.x, c.y);

        if(c.x != x || c.y+1 != y)
            TryAddToQueue(s, queue, r, dist, c.x, c.y + 1);
        if(c.x != x || c.y-1 != y)
            TryAddToQueue(s, queue, r, dist, c.x, c.y - 1);
        if(c.x+1 != x || c.y != y)
            TryAddToQueue(s, queue, r, dist, c.x + 1, c.y);
        if(c.x-1 != x || c.y != y)
            TryAddToQueue(s, queue, r, dist, c.x - 1, c.y);


    }

    return 0;
}

///////////////////////
// General Functions //
///////////////////////

void PrintMap(RMap &r)
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            std::cout << (r.GetDistance(j, i) >= 10 ? "" : " ");
            std::cout << r.GetDistance(j, i) << " ";
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

}
