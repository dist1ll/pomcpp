#include <random>

#include "catch.hpp"
#include "bboard.hpp"
#include "strategy.hpp"

using namespace bboard;

TEST_CASE("IsAdjacent", "[strategy]")
{
    std::unique_ptr<bboard::State> s = std::make_unique<bboard::State>();

    SECTION("Enemy Close")
    {
        s->PutAgent(5, 5, 0);
        s->PutAgent(4, 4, 1);
        REQUIRE(strategy::IsAdjacentEnemy(*s.get(), 0, 2));
        REQUIRE(strategy::IsAdjacentEnemy(*s.get(), 0, 3));
    }
    SECTION("No Enemy Close")
    {
        s->PutAgent(5, 5, 0);
        s->PutAgent(3, 2, 1);
        for(int i = 0; i < 5; i++)
        {
            REQUIRE(!strategy::IsAdjacentEnemy(*s.get(), 0, i));
        }
    }
}

TEST_CASE("Fill RMap", "[strategy]")
{
    std::unique_ptr<State> s = std::make_unique<State>();
    InitBoardItems(*s.get(), 0x13327);

    strategy::RMap r;

    s->Kill(1, 2, 3);
    s->PutAgent(0, 0, 0);
    strategy::FillRMap(*s.get(), r, 0);

    // Can confirm by printing:

    // PrintState(s.get());
    // strategy::PrintMap(r);
    // strategy::PrintPath(r, {0,0}, {8,10});

    for(int y = 0; y < BOARD_SIZE; y++)
    {
        for(int x = 0; x < BOARD_SIZE; x++)
        {
            if(s->board[y][x] == Item::RIGID)
            {
                REQUIRE(!strategy::IsReachable(r, x, y));
            }

        }
    }
}

TEST_CASE("Move Towards Methods", "[strategy]")
{
    std::unique_ptr<State> s = std::make_unique<State>();
    InitBoardItems(*s.get(), 0x1337);

    strategy::RMap r;

    SECTION("MoveTowardsPosition")
    {
        s->Kill(1, 2, 3);
        s->PutAgent(4, 5, 0);
        strategy::FillRMap(*s.get(), r, 0);
        Move m1 = strategy::MoveTowardsPosition(r, {4, 1});
        Move m2 = strategy::MoveTowardsPosition(r, {3, 6});
        Move m3 = strategy::MoveTowardsPosition(r, {0,10});

        REQUIRE(m1 == Move::UP);
        REQUIRE(m2 == Move::DOWN);
        REQUIRE(m3 == Move::DOWN);
    }
    SECTION("MoveTowardsPowerup")
    {
        s->Kill(1, 2, 3);
        s->PutAgent(4, 5, 0);
        s->PutItem(2, 6, Item::EXTRABOMB);

        strategy::FillRMap(*s.get(), r, 0);

        Move m1 = strategy::MoveTowardsPowerup(*s.get(), r, 2);
        Move m2 = strategy::MoveTowardsPowerup(*s.get(), r, 3);

        REQUIRE(m1 == Move::IDLE);
        REQUIRE(m2 == Move::DOWN);
    }
    SECTION("MoveTowardsEnemy")
    {
        s->Kill(2, 3);
        s->PutAgent(4, 5, 0);
        s->PutAgent(2, 6, 1);

        strategy::FillRMap(*s.get(), r, 0);

        Move m1 = strategy::MoveTowardsEnemy(*s.get(), r, 2);
        Move m2 = strategy::MoveTowardsEnemy(*s.get(), r, 3);

        REQUIRE(m1 == Move::IDLE);
        REQUIRE(m2 == Move::DOWN);
    }
}
