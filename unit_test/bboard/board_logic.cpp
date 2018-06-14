#include "catch.hpp"
#include "bboard.hpp"


bool IsAgentPos(bboard::State* state, int agent, int x, int y)
{
    int o = bboard::Item::AGENT0 + agent;
    return  state->agentX[agent] == x &&
            state->agentY[agent] == y && state->board[y][x] == o;
}
TEST_CASE( "Basic Non-Obstacle Movement", "[step function]" )
{
    bboard::State* s = bboard::InitEmpty(0, 1, 2, 3);
    bboard::Move* m = new bboard::Move[4]();

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 1, 0));

    m[0] = bboard::Move::DOWN;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 1, 1));

    m[0] = bboard::Move::LEFT;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 0, 1));

    m[0] = bboard::Move::UP;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 0, 0));
    delete s;
}

TEST_CASE( "Basic Obstacle Collision", "[step function]" )
{
    bboard::State* s = bboard::InitEmpty(0, 1, 2, 3);
    bboard::Move* m = new bboard::Move[4]();

    s->PutItem(0, 1, bboard::Item::RIGID);

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 0, 0));

    m[0] = bboard::Move::DOWN;
    bboard::Step(s, m);
    REQUIRE(IsAgentPos(s, 0, 0, 1));

    delete s;
}
