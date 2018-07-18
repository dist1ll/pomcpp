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

TEST_CASE("Can Safely Bomb", "[strategy]")
{

}
