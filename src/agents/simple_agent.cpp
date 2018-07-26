#include "bboard.hpp"
#include "agents.hpp"

namespace agents
{


bboard::Move SimpleAgent::act(const bboard::State* state)
{
    return bboard::Move::IDLE;
}

}

