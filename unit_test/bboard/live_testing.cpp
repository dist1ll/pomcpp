#include "catch.hpp"
#include "bboard.hpp"
#include "agents.hpp"

using namespace agents;

template <int AGENT>
void PrintAgentInfo(const bboard::State& state)
{
    std::cout << std::endl;
    std::cout << "debbuging agent nr. " << AGENT;
}

TEST_CASE("Test Simple Agent", "[live testing]")
{
    SimpleAgent simpleton;
    std::array<LazyAgent, 3> r;

    // create an environment
    bboard::Environment e;

    // invokes function after every step for debugging
    e.SetStepListener(PrintAgentInfo<0>);

    // initializes the game/board/agents
    e.MakeGame({&simpleton, &r[0], &r[1], &r[2]});

    // starts the game with the specified params
    e.StartGame(50, true, true);
}
