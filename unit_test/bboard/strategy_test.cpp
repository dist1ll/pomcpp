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
    strategy::FillRMap(*s.get(), r, 0, 0);

    // Can confirm by printing:

    // PrintState(s.get());
    // strategy::PrintMap(r);

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
