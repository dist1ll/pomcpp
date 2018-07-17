#include "catch.hpp"
#include "bboard.hpp"
#include "strategy.hpp"

TEST_CASE("IsAdjacent", "[strategy]")
{
    std::unique_ptr<bboard::State> s = std::make_unique<bboard::State>();

    SECTION("Enemy Close")
    {
    }
    SECTION("No Enemy Close")
    {

    }
    SECTION("Player In Corner")
    {

    }
}
