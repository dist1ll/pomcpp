#include "catch.hpp"
#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"

using namespace agents;

template <int AGENT>
void PrintAgentInfo(const bboard::Environment& env)
{
    SimpleAgent& a = *static_cast<SimpleAgent*>(env.GetAgent(AGENT));
    std::cout << std::endl;
    // std::cout << a.danger;
    bboard::strategy::PrintMap(a.r);
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
    e.StartGame(500, true, false);
}
