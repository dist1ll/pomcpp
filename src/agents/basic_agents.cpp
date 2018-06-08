#include <random>

#include "bboard.hpp"
#include "agents.hpp"

namespace agents
{


bboard::Move RandomAgent::act(bboard::State* state)
{
    return static_cast<bboard::Move>(intDist(rng));
}

bboard::Move LazyAgent::act(bboard::State* state)
{
    return bboard::Move::IDLE;
}

}
