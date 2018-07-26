#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"

using namespace bboard;
using namespace bboard::strategy;

namespace agents
{

SimpleAgent::SimpleAgent()
{
    std::random_device rd;  // non explicit seed
    rng = std::mt19937_64(rd());
    intDist = std::uniform_int_distribution<int>(0, 5);
}

Move SimpleAgent::act(const State* state)
{
    const AgentInfo& a = state->agents[id];
    FillRMap(*state, r, id);

    danger = strategy::IsInDanger(*state, id);


    if(danger > 0)
    {
        return MoveTowardsSafePlace(*state, r, danger);
    }
    else
    {
        if(intDist(rng))
        {
            return static_cast<bboard::Move>(intDist(rng));
        }
        else
        {
            return Move::BOMB;
        }
    }
}

}

