#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"

using namespace bboard;
using namespace bboard::strategy;

namespace agents
{

Move SimpleAgent::act(const State* state)
{
    FillRMap(*state, r, id);

    return Move::RIGHT;
}

}

