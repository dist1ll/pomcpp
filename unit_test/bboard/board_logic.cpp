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
    REQUIRE(state->agents[agent].x == x);
    REQUIRE(state->agents[agent].y == y);
    REQUIRE(state->board[y][x] == o);
}

bool IsAgentPos(bboard::State* state, int agent, int x, int y)
{
    int o = bboard::Item::AGENT0 + agent;
    return  state->agents[agent].x == x &&
            state->agents[agent].y == y && state->board[y][x] == o;
}

void PlaceBrick(bboard::State* state, int x, int y)
{
    state->board[y][x] = bboard::Item::RIGID;
}

TEST_CASE("Basic Non-Obstacle Movement", "[step function]")
{
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();
    s->PutAgentsInCorners(0, 1, 2, 3);

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
}

TEST_CASE("Basic Obstacle Collision", "[step function]")
{
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();
    s->PutAgentsInCorners(0, 1, 2, 3);

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutItem(1, 0, bboard::Item::RIGID);

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 0);

    m[0] = bboard::Move::DOWN;
    bboard::Step(s, m);
    REQUIRE_AGENT(s, 0, 0, 1);
}

TEST_CASE("Destination Collision", "[step function]")
{
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();

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
}

TEST_CASE("Movement Dependency Handling", "[step function]")
{
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();

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
    SECTION("Two On One")
    {
        /* For clarity:
         * 0 -> 2 <- 1
         *      |
         *      3
         */

        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 2, 0);
        s->PutAgent(2, 1, 0);
        s->PutAgent(3, 1, 1);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::LEFT;
        m[2] = m[3] = bboard::Move::DOWN;

        bboard::Step(s, m);
        REQUIRE_AGENT(s, 0, 0, 0);
        REQUIRE_AGENT(s, 1, 2, 0);
        REQUIRE_AGENT(s, 2, 1, 1);
        REQUIRE_AGENT(s, 3, 1, 2);
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
}

TEST_CASE("Bomb Mechanics", "[step function]")
{
    std::unique_ptr<bboard::State> s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    SECTION("Standard Bomb Laying")
    {
        s->PutAgentsInCorners(0, 1, 2, 3);
        m[0] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);
        REQUIRE(s->board[0][0] == bboard::Item::AGENT0);

        m[0] = bboard::Move::DOWN;
        bboard::Step(s.get(), m);
        REQUIRE(s->board[0][0] == bboard::Item::BOMB);
    }
    SECTION("Bomb Movement Block Simple")
    {
        s->PutAgentsInCorners(0, 1, 2, 3);
        s->PutItem(1, 0, bboard::Item::BOMB);

        m[0] = bboard::Move::RIGHT;
        bboard::Step(s.get(), m);
        REQUIRE_AGENT(s.get(), 0, 0, 0);
    }
    SECTION("Bomb Movement Block Complex")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 2, 0);
        s->PutAgent(3, 3, 0);

        m[0] = m[1] = m[2] = bboard::Move::RIGHT;
        m[3] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);
        REQUIRE_AGENT(s.get(), 0, 0, 0);
        REQUIRE_AGENT(s.get(), 1, 1, 0);
        REQUIRE_AGENT(s.get(), 2, 2, 0);

        m[0] = m[1] = m[2] = bboard::Move::IDLE;
        m[3] = bboard::Move::RIGHT;
        bboard::Step(s.get(), m);
        REQUIRE_AGENT(s.get(), 3, 4, 0);
    }
    SECTION("Bomb Ouroboros Block")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 1, 1);
        s->PutAgent(3, 0, 1);

        m[0] = m[1] = m[2] = m[3] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::DOWN;
        m[2] = bboard::Move::LEFT;
        m[3] = bboard::Move::UP;
        bboard::Step(s.get(), m);

        //everyone planted bombs, you can't move
        REQUIRE_AGENT(s.get(), 0, 0, 0);
        REQUIRE_AGENT(s.get(), 1, 1, 0);
        REQUIRE_AGENT(s.get(), 2, 1, 1);
        REQUIRE_AGENT(s.get(), 3, 0, 1);
    }
}

TEST_CASE("Bomb Explosion", "[step function]")
{

}
