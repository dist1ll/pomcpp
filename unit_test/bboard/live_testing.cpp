#include "catch.hpp"
#include "bboard.hpp"
#include "agents.hpp"

using namespace agents;

TEST_CASE("Test Simple Agent", "[live testing]")
{
    bboard::Environment e;
    std::array<SimpleAgent, 4> r;
    e.MakeGame({&r[0], &r[1], &r[2], &r[3]});
    e.StartGame(5, true, true);
}
