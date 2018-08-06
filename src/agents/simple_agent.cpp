#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"
#include "step_utility.hpp"

using namespace bboard;
using namespace bboard::strategy;

namespace agents
{

bool _CheckPos(const State& state, int x, int y)
{
    return !util::IsOutOfBounds(x, y) && IS_WALKABLE(state.board[y][x]);
}

SimpleAgent::SimpleAgent()
{
    std::random_device rd;  // non explicit seed
    rng = std::mt19937_64(rd());
    intDist = std::uniform_int_distribution<int>(0, 4); // no bombs
}


Move _Decide(SimpleAgent& me, const State* state)
{
    const AgentInfo& a = state->agents[me.id];
    FillRMap(*state, me.r, me.id);

    me.danger = IsInDanger(*state, me.id);

    if(me.danger > 0)
    {
        return MoveTowardsSafePlace(*state, me.r, me.danger);
    }

    if(a.bombCount < a.maxBombCount)
    {
        if(IsAdjacentEnemy(*state, me.id, 2)
                || IsAdjacentItem(*state, me.id, 1, Item::WOOD))
        {
            return Move::BOMB;
        }

        if(IsAdjacentEnemy(*state, me.id, 7))
        {
            return MoveTowardsEnemy(*state, me.r, 7);
        }
    }
    me.moveQueue.count = 0;
    SafeDirections(*state, me.moveQueue, a.x, a.y);
    SortDirections(me.moveQueue, me.recentPositions, a.x, a.y);

    if(me.moveQueue.count == 0)
    {
        return Move::IDLE;
    }
    else
    {
        return me.moveQueue[me.intDist(me.rng) % 2];
    }
}
Move SimpleAgent::act(const State* state)
{
    const AgentInfo& a = state->agents[id];
    Move m = _Decide(*this, state);
    Position p = util::DesiredPosition(a.x, a.y, m);

    if(recentPositions.RemainingCapacity() == 0)
    {
        recentPositions.PopElem();
    }
    recentPositions.AddElem(p);

    return m;
}

void SimpleAgent::PrintDetailedInfo()
{
    for(int i = 0; i < recentPositions.count; i++)
    {
        std::cout << recentPositions[i] << std::endl;
    }
}

}

