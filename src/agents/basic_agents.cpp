#include <random>

#include "bboard.hpp"
#include "agents.hpp"

namespace agents
{

////////////////////
//  Random Agent  //
////////////////////
RandomAgent::RandomAgent()
{
    std::random_device rd;  // non explicit seed
    rng = std::mt19937_64(rd());
    intDist = std::uniform_int_distribution<int>(0, 5);
}

bboard::Move RandomAgent::act(bboard::State* state)
{
    return static_cast<bboard::Move>(intDist(rng));
}

//////////////////////
//  Harmless Agent  //
//////////////////////
HarmlessAgent::HarmlessAgent()
{
    std::random_device rd;  // non explicit seed
    rng = std::mt19937_64(rd());
    intDist = std::uniform_int_distribution<int>(0, 4); // 6 is bomb
}

bboard::Move HarmlessAgent::act(bboard::State* state)
{
    return static_cast<bboard::Move>(intDist(rng));
}

//////////////////
//  Lazy Agent  //
//////////////////
bboard::Move LazyAgent::act(bboard::State* state)
{
    return bboard::Move::IDLE;
}

}
