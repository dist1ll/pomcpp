#include "bboard.hpp"
#include "agents.hpp"

namespace agents
{


bboard::Move RandomAgent::act(bboard::State* state)
{
    return bboard::Move::DOWN; // TODO: randomize
}

bboard::Move LazyAgent::act(bboard::State* state)
{
    return bboard::Move::IDLE;
}

}
