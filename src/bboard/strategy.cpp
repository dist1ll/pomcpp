#include <unordered_set>

#include "bboard.hpp"
#include "colors.hpp"
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
inline RMapInfo TryAdd(State& s, FixedQueue<T, N>& q, RMap& r, Position& c, int cx, int cy)
{
    int dist = r.GetDistance(c.x, c.y);
    if(!util::IsOutOfBounds(cx, cy) &&
            r.GetDistance(cx, cy) == 0 &&
            s.board[cy][cx] == Item::PASSAGE)
    {
        r.SetPredecessor(cx, cy, c.x, c.y);
        r.SetDistance(cx, cy, dist + 1);
        q.AddElem({cx, cy});
        return 0;
    }
    return 0;
}
// BFS
RMapInfo FillRMap(State& s, RMap& r, int agentID)
{
    int x = s.agents[agentID].x;
    int y = s.agents[agentID].y;

    FixedQueue<Position, BOARD_SIZE * BOARD_SIZE> queue;
    r.SetDistance(x, y, 0);
    queue.AddElem({x, y});

    AgentInfo& a = s.agents[agentID];
    RMapInfo result = 0;

    while(queue.count != 0)
    {

        Position& c = queue.PopElem();
        int dist = r.GetDistance(c.x, c.y);
        if(IsInBombRange(a.x, a.y, a.bombStrength, c) && dist < 10)
        {
            result |= 0b1;
        }

        if(c.x != x || c.y+1 != y)
            TryAdd(s, queue, r, c, c.x, c.y + 1);
        if(c.x != x || c.y-1 != y)
            TryAdd(s, queue, r, c, c.x, c.y - 1);
        if(c.x+1 != x || c.y != y)
            TryAdd(s, queue, r, c, c.x + 1, c.y);
        if(c.x-1 != x || c.y != y)
            TryAdd(s, queue, r, c, c.x - 1, c.y);

    }
    r.info = result;
    return result;
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


void PrintPath(RMap &r, Position from, Position to)
{
    std::array<Position, BOARD_SIZE * BOARD_SIZE> path;
    std::unordered_set<Position> pathx;

    path[0] = {to.x, to.y};
    Position curr = path[0];

    for(uint i = 0; !(curr == from); i++)
    {
        pathx.insert(curr);
        int idx = r.GetPredecessor(curr.x, curr.y);
        int y = idx / BOARD_SIZE;
        int x = idx % BOARD_SIZE;
        curr = path[i] = {x, y};
    }

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            std::cout << (r.GetDistance(j, i) >= 10 ? "" : " ");
            auto dist = std::to_string(r.GetDistance(j, i));
            std::cout << (pathx.count({j, i}) ? FRED(dist) : dist) << " ";
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
