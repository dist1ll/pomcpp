#include "catch.hpp"
#include "bboard.hpp"
#include "agents.hpp"

TEST_CASE("Default Scenario", "[live testing]")
{
    bboard::State* init = bboard::InitState(0,1,2,3);

    init->PutItem(1, 4, bboard::Item::INCRRANGE);
    init->PutItem(6, 4, bboard::Item::KICK);
    init->PutItem(7, 6, bboard::Item::EXTRABOMB);

    agents::RandomAgent r;
    bboard::Agent* agents[4] = {&r, &r, &r, &r};
    bboard::StartGame(init, agents, 500);
    delete init;
}
