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

Move SimpleAgent::act(const State* state)
{
    const AgentInfo& a = state->agents[id];
    FillRMap(*state, r, id);

    danger = IsInDanger(*state, id);

    if(danger > 0)
    {
        return MoveTowardsSafePlace(*state, r, danger);
    }

    if(a.bombCount < a.maxBombCount)
    {
        if(IsAdjacentEnemy(*state, id, 1)
                || IsAdjacentItem(*state, id, 1, Item::WOOD))
        {
            return Move::BOMB;
        }

        if(IsAdjacentEnemy(*state, id, 3))
        {
            return MoveTowardsEnemy(*state, r, 3);
        }
    }
    moveQueue.count = 0;
    SafeDirections(*state, moveQueue, a.x, a.y);

    if(moveQueue.count == 0)
    {
        return Move::IDLE;
    }
    else
    {
        return moveQueue[intDist(rng) % moveQueue.count];
    }
}

}

