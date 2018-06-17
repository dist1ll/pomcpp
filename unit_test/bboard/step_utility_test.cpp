#include <iostream>

#include "catch.hpp"
#include "bboard.hpp"
#include "step_utility.hpp"


void REQUIRE_POS(bboard::Position* p, int idx, int x, int y)
{
    REQUIRE(p[idx].x == x);
    REQUIRE(p[idx].y == y);
}
TEST_CASE("Destination position filling", "[step utilities]")
{
    bboard::State* s = new bboard::State();
    bboard::Move m[4] =
    {
        bboard::Move::DOWN, bboard::Move::LEFT,
        bboard::Move::RIGHT, bboard::Move::UP
    };

    s->PutAgent(0, 0, 0);
    s->PutAgent(1, 1, 0);
    s->PutAgent(2, 2, 0);
    s->PutAgent(3, 3, 0);

    bboard::Position destPos[4];
    bboard::FillDestPos(s, m, destPos);

    REQUIRE_POS(destPos, 0, 0, 1);
    REQUIRE_POS(destPos, 1, 0, 0);
    REQUIRE_POS(destPos, 2, 3, 0);
    REQUIRE_POS(destPos, 3, 3, -1);

    delete s;
}

TEST_CASE("Dependency Resolving", "[step utilities]")
{
    bboard::State* s = new bboard::State();
    bboard::Move idle = bboard::Move::IDLE;
    bboard::Move m[4] = {idle, idle, idle, idle};
    bboard::Position dest[4];
    int dependency[4] = {-1, -1, -1, -1};
    int chain[4] = {-1, -1, -1, -1};

    SECTION("Resolve 0->1 dependency")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 9, 9);
        s->PutAgent(3, 8, 8);

        m[0] = m[1] = bboard::Move::RIGHT;
        bboard::FillDestPos(s, m, dest);

        bboard::ResolveDependencies(s, dest, dependency, chain);

        bboard::PrintDependency(dependency);
        bboard::PrintDependencyChain(dependency, chain);

    }
    delete s;
}
