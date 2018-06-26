#include <iostream>

#include "catch.hpp"
#include "bboard.hpp"

/**
 * @brief REQUIRE_AGENT Proxy REQUIRE-Assertion to test
 * valid position on board AND agent arrays.
 */
void REQUIRE_AGENT(bboard::State* state, int agent, int x, int y)
{
    int o = bboard::Item::AGENT0 + agent;
    REQUIRE(state->agentX[agent] == x);
    REQUIRE(state->agentY[agent] == y);
    REQUIRE(state->board[y][x] == o);
}

bool IsAgentPos(bboard::State* state, int agent, int x, int y)
{
    int o = bboard::Item::AGENT0 + agent;
    return  state->agentX[agent] == x &&
            state->agentY[agent] == y && state->board[y][x] == o;
}

void PlaceBrick(bboard::State* state, int x, int y)
{
    state->board[y][x] = bboard::Item::RIGID;
}

TEST_CASE("Basic Non-Obstacle Movement", "[step function]")
{
    bboard::State* s = bboard::InitEmpty(0, 1, 2, 3);
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 1, 0);

    m[0] = bboard::Move::DOWN;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 1, 1);

    m[0] = bboard::Move::LEFT;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 1);

    m[0] = bboard::Move::UP;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 0);

    m[3] = bboard::Move::UP;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 3, 0, 9);
    delete s;
}

TEST_CASE("Basic Obstacle Collision", "[step function]")
{
    bboard::State* s = bboard::InitEmpty(0, 1, 2, 3);

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutItem(0, 1, bboard::Item::RIGID);

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 0);

    m[0] = bboard::Move::DOWN;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 1);

    delete s;
}

TEST_CASE("Destination Collision", "[step function]")
{
    bboard::State* s = new bboard::State();

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutAgent(0, 0, 1);
    s->PutAgent(1, 2, 1);

    s->Kill(2, 3);

    SECTION("Two Agent-Collision")
    {
        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::LEFT;

        bboard::Step(s, m);

        REQUIRE_AGENT(s, 0, 0, 1);
        REQUIRE_AGENT(s, 1, 2, 1);
    }
    SECTION("Dead Collision")
    {
        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::LEFT;
        s->Kill(1);

        bboard::Step(s, m);

        REQUIRE_AGENT(s, 0, 1, 1);
    }
    SECTION("Four Agent-Collision")
    {
        s->PutAgent(2, 1, 0);
        s->PutAgent(3, 1, 2);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::LEFT;
        m[2] = bboard::Move::DOWN;
        m[3] = bboard::Move::UP;

        bboard::Step(s, m);

        REQUIRE_AGENT(s, 0, 0, 1);
        REQUIRE_AGENT(s, 1, 2, 1);
        REQUIRE_AGENT(s, 2, 1, 0);
        REQUIRE_AGENT(s, 3, 1, 2);
    }
    delete s;
}

TEST_CASE("Movement Dependency Handling", "[step function]")
{
    bboard::State* s = new bboard::State();

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    SECTION("Move Chain Against Obstacle")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 2, 0);
        s->PutAgent(3, 3, 0);

        s->PutItem(4, 0, bboard::Item::RIGID);

        m[0] = m[1] = m[2] = m[3] = bboard::Move::RIGHT;

        bboard::Step(s, m);
        REQUIRE_AGENT(s, 0, 0, 0);
        REQUIRE_AGENT(s, 1, 1, 0);
        REQUIRE_AGENT(s, 2, 2, 0);
        REQUIRE_AGENT(s, 3, 3, 0);
    }
    SECTION("Move Ouroboros")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 1, 1);
        s->PutAgent(3, 0, 1);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::DOWN;
        m[2] = bboard::Move::LEFT;
        m[3] = bboard::Move::UP;

        bboard::Step(s, m);
        REQUIRE_AGENT(s, 3, 0, 0);
        REQUIRE_AGENT(s, 0, 1, 0);
        REQUIRE_AGENT(s, 1, 1, 1);
        REQUIRE_AGENT(s, 2, 0, 1);
    }

    delete s;
}
