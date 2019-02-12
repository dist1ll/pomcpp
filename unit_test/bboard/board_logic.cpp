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

/**
 * @brief SeveralSteps Proxy for bboard::Step, useful for testing
 */
void SeveralSteps(int times, bboard::State* s, bboard::Move* m)
{
    for(int i = 0; i < times; i++)
    {
        bboard::Step(s, m);
    }
}

/**
 * @brief PlaceBombsDiagonally Lets an agent plant bombs along a
 * horizontal
 */
void PlaceBombsHorizontally(bboard::State* s, int agent, int bombs)
{
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[bboard::AGENT_COUNT] = {id, id, id, id};

    for(int i = 0; i < bombs; i++)
    {
        m[agent] = bboard::Move::BOMB;
        bboard::Step(s, m);
        m[agent] = bboard::Move::RIGHT;
        bboard::Step(s, m);
    }
}

bool IsAgentPos(bboard::State* state, int agent, int x, int y)
{
    int o = bboard::Item::AGENT0 + agent;
    return  state->agents[agent].x == x &&
            state->agents[agent].y == y && state->board[y][x] == o;
}

TEST_CASE("Basic Non-Obstacle Movement", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    s->PutAgentsInCorners(0, 1, 2, 3);

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 1, 0);

    m[0] = bboard::Move::DOWN;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 1, 1);

    m[0] = bboard::Move::LEFT;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 0, 1);

    m[0] = bboard::Move::UP;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 0, 0);

    m[3] = bboard::Move::UP;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 3, 0, 9);

}

TEST_CASE("Basic Obstacle Collision", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    s->PutAgentsInCorners(0, 1, 2, 3);

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutItem(1, 0, bboard::Item::RIGID);

    m[0] = bboard::Move::RIGHT;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 0, 0);

    m[0] = bboard::Move::DOWN;
    bboard::Step(s.get(), m);
    REQUIRE_AGENT(s.get(), 0, 0, 1);
}

TEST_CASE("Movement Against Flames", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutAgentsInCorners(0, 1, 2, 3);
    s->SpawnFlame(1,1,2);

    m[0] = bboard::Move::RIGHT;

    bboard::Step(s.get(), m);

    REQUIRE(s->agents[0].dead);
    REQUIRE(s->board[0][0] == bboard::Item::PASSAGE);
}

TEST_CASE("Destination Collision", "[step function]")
{
    auto sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutAgent(0, 1, 0);
    s->PutAgent(2, 1, 1);

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
        s->PutAgent(1, 0, 2);
        s->PutAgent(1, 2, 3);

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
    auto sx = std::make_unique<bboard::State>();
    bboard::State* s = sx.get();

    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    SECTION("Move Chain Against Obstacle")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 0, 1);
        s->PutAgent(2, 0, 2);
        s->PutAgent(3, 0, 3);

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
        s->PutAgent(2, 0, 1);
        s->PutAgent(1, 0, 2);
        s->PutAgent(1, 1, 3);

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
        s->PutAgent(1, 0, 1);
        s->PutAgent(1, 1, 2);
        s->PutAgent(0, 1, 3);

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
    auto s = std::make_unique<bboard::State>();
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
        s->PutAgent(1, 0, 1);
        s->PutAgent(2, 0, 2);
        s->PutAgent(3, 0, 3);

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
        s->PutAgent(1, 0, 1);
        s->PutAgent(1, 1, 2);
        s->PutAgent(0, 1, 3);

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
    auto s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->Kill(2,3);
    s->PutAgent(5, 5, 0); //

    SECTION("Bomb Goes Off Correctly")
    {
        m[0] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[0] = bboard::Move::UP;
        SeveralSteps(bboard::BOMB_LIFETIME - 1, s.get(), m);

        REQUIRE(s->board[5][5] == bboard::Item::BOMB);
        bboard::Step(s.get(), m);
        REQUIRE(IS_FLAME(s->board[5][5]));
    }
    SECTION("Destroy Objects and Agents")
    {
        s->PutItem(6, 5, bboard::Item::WOOD);
        s->PutAgent(4, 5, 1);

        m[0] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[0] = bboard::Move::UP;
        SeveralSteps(bboard::BOMB_LIFETIME, s.get(), m);

        REQUIRE(s->agents[1].dead);
        REQUIRE(IS_FLAME(s->board[5][4]));
        REQUIRE(IS_FLAME(s->board[5][6]));
    }
    SECTION("Keep Rigid")
    {
        s->PutItem(6, 5, bboard::Item::RIGID);

        m[0] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[0] = bboard::Move::UP;
        SeveralSteps(bboard::BOMB_LIFETIME, s.get(), m);

        REQUIRE(s->board[5][6] == bboard::Item::RIGID);
    }
    SECTION("Kill Only 1 Wood")
    {
        s->PutItem(7, 5, bboard::Item::WOOD);
        s->PutItem(8, 5, bboard::Item::WOOD);

        s->agents[0].bombStrength = 5;
        s->PlantBomb(6, 5, 0, true);
        SeveralSteps(bboard::BOMB_LIFETIME, s.get(), m);

        REQUIRE(IS_FLAME(s->board[5][7]));
        REQUIRE(!IS_FLAME(s->board[5][8]));
    }
    SECTION("Max Agent Bomb Limit")
    {
        s->agents[0].maxBombCount = 2;
        REQUIRE(s->agents[0].bombCount == 0);

        PlaceBombsHorizontally(s.get(), 0, 4); //place 1 over max
        REQUIRE(s->board[5][5] == bboard::Item::BOMB);
        REQUIRE(s->board[5][6] == bboard::Item::BOMB);
        REQUIRE(s->board[5][7] == bboard::Item::PASSAGE);

        REQUIRE(s->agents[0].bombCount == 2);
    }
}

TEST_CASE("Flame Mechanics", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};
    s->PutAgentsInCorners(0, 1, 2, 3);


    SECTION("Correct Lifetime Calculation")
    {
        s->SpawnFlame(5,5,4);
        bboard::Step(s.get(), m);

        SeveralSteps(bboard::FLAME_LIFETIME - 2, s.get(), m);
        REQUIRE(IS_FLAME(s->board[5][5]));
        bboard::Step(s.get(), m);
        REQUIRE(!IS_FLAME(s->board[5][5]));
    }
    SECTION("Vanish Flame Completely")
    {
        s->SpawnFlame(5,5,4);
        bboard::Step(s.get(), m);

        for(int i = 0; i <= 4; i++)
        {
            REQUIRE(IS_FLAME(s->board[5][5 + i]));
            REQUIRE(IS_FLAME(s->board[5][5 - i]));
            REQUIRE(IS_FLAME(s->board[5 + i][5]));
            REQUIRE(IS_FLAME(s->board[5 - i][5]));
        }
    }
    SECTION("Only Vanish Your Own Flame")
    {
        s->SpawnFlame(5,5,4);
        bboard::Step(s.get(), m);

        s->SpawnFlame(6, 6, 4);
        SeveralSteps(bboard::FLAME_LIFETIME - 1, s.get(), m);

        REQUIRE(IS_FLAME(s->board[5][6]));
        REQUIRE(IS_FLAME(s->board[6][5]));
        REQUIRE(!IS_FLAME(s->board[5][5]));
    }
}

TEST_CASE("Chained Explosions", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};
    /*
     * Note two ways to plant bombs in this test
     * PlantBomb  =>  Bomb gets ticked
     * Set Move to Bomb  =>  Bomb doesn't get ticked
     */

    SECTION("Two Bombs")
    {
        s->PutAgentsInCorners(0, 1, 2, 3);
        s->PlantBomb(5, 5, 0, true);
        bboard::Step(s.get(), m);
        s->PlantBomb(4, 5, 1, true);
        SeveralSteps(bboard::BOMB_LIFETIME - 1, s.get(), m);
        REQUIRE(s->bombs.count == 0);
        REQUIRE(IS_FLAME(s->board[5][6]));
    }
    SECTION("Two Bombs Covered By Agent")
    {
        s->PutAgent(5, 5, 0);
        s->PutAgent(4, 5, 1);
        s->Kill(2, 3);
        m[0] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[1] = bboard::Move::BOMB;
        bboard::Step(s.get(), m);

        m[0] = m[1] = bboard::Move::DOWN;

        SeveralSteps(bboard::BOMB_LIFETIME - 2, s.get(), m);

        REQUIRE(s->bombs.count == 2);
        bboard::Step(s.get(), m);
        REQUIRE(s->bombs.count == 0);
        REQUIRE(s->flames.count == 2);
    }


}

TEST_CASE("Bomb Kick Mechanics", "[step function]")
{
    auto s = std::make_unique<bboard::State>();
    bboard::Move id = bboard::Move::IDLE;
    bboard::Move m[4] = {id, id, id, id};

    s->PutAgent(0, 1, 0);
    s->agents[0].canKick = true;
    s->PlantBomb(1, 1, 0, true);
    s->agents[0].maxBombCount = bboard::MAX_BOMBS_PER_AGENT;
    m[0] = bboard::Move::RIGHT;

    SECTION("One Agent - One Bomb")
    {
        s->Kill(1, 2, 3);
        bboard::Step(s.get(), m);

        REQUIRE_AGENT(s.get(), 0, 1, 1);
        REQUIRE(s->board[1][2] == bboard::Item::BOMB);

        for(int i = 0; i < 4; i++)
        {
            REQUIRE(s->board[1][2 + i] == bboard::Item::BOMB);
            bboard::Step(s.get(), m);
            m[0] = bboard::Move::IDLE;
        }
    }
    SECTION("Bomb kicked against Flame")
    {
        s->Kill(1, 2, 3);
        s->PutItem(5, 1, bboard::Item::FLAMES);

        bboard::Step(s.get(), m);
        m[0] = bboard::Move::IDLE;

        SeveralSteps(3,  s.get(),  m);

        REQUIRE(IS_FLAME(s->board[1][5]));
        REQUIRE(s->bombs.count == 0);
        REQUIRE(s->flames.count == 1);
        REQUIRE(s->flames[0].position == bboard::Position({5,1}));
    }
    SECTION("Bomb - Bomb Collision")
    {
        s->Kill(1, 2, 3);
        s->PlantBomb(7, 7, 0, true);
        bboard::SetBombDirection(s->bombs[1], bboard::Direction::UP);

        for(int i = 0; i < 6; i++)
        {
            bboard::Step(s.get(), m);
            m[0] = bboard::Move::IDLE;
        }

        REQUIRE(BMB_POS_X(s->bombs[0]) == 6);
        REQUIRE(BMB_POS_X(s->bombs[1]) == 7);
        REQUIRE(BMB_POS_Y(s->bombs[1]) == 2);
    }


    /*


        SECTION("One Agent - One Bomb")
        {
            s->Kill(1, 2, 3);
            bboard::PrintState(s.get(), true);
            std::cin.get();

            for(int i = 0; i < 14; i++)
            {
                bboard::Step(s.get(), m);
                bboard::PrintState(s.get(), true);
                std::cin.get();
                m[0] = bboard::Move::IDLE;
            }
        }*/
}
