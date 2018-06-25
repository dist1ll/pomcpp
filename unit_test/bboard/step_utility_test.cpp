#include <iostream>

#include "catch.hpp"
#include "bboard.hpp"
#include "step_utility.hpp"


template<typename... Args>
void REQUIRE_ROOTS(int c[4]) { }

template<typename... Args>
void REQUIRE_ROOTS(int c[4], int root, Args... args)
{
    int fit = -1;
    for(int i = 0; i < 4; i++)
    {
        if(root == c[i])
        {
            fit = c[i];
        }
    }
    REQUIRE(fit == root);
    REQUIRE_ROOTS(c, args...);
}

void REQUIRE_POS(bboard::Position* p, int idx, int x, int y)
{
    REQUIRE(p[idx].x == x);
    REQUIRE(p[idx].y == y);
}

void REQUIRE_POS(bboard::Position p, int x, int y)
{
    bboard::Position q = {x, y};
    REQUIRE((p == q));
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

TEST_CASE("Fix Switch Position", "[step utilities]")
{
    bboard::State* s = new bboard::State();
    bboard::Position des[4];
    bboard::Move r = bboard::Move::RIGHT, l = bboard::Move::LEFT;
    bboard::Move m[4] = {r, r, l, l};

    s->PutAgent(0, 0, 0);
    s->PutAgent(1, 1, 0);
    s->PutAgent(2, 2, 0);
    s->PutAgent(3, 3, 0);

    bboard::FillDestPos(s, m, des);
    bboard::FixSwitchMove(s, des);

    REQUIRE_POS(des[0], 1, 0);
    REQUIRE_POS(des[1], s->agentX[1], s->agentY[1]);
    REQUIRE_POS(des[2], s->agentX[2], s->agentY[2]);
    REQUIRE_POS(des[3], 2, 0);
    delete s;
}

TEST_CASE("Dependency Resolving", "[step utilities]")
{
    bboard::State* s = new bboard::State();
    bboard::Move idle = bboard::Move::IDLE;
    bboard::Move m[4] = {idle, idle, idle, idle};
    bboard::Position des[4];
    int dependency[4] = {-1, -1, -1, -1};
    int chain[4] = {-1, -1, -1, -1};

    SECTION("Resolve 0->1 Dependency")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 8, 4);
        s->PutAgent(3, 9, 8);

        m[0] = m[1] = m[2] = bboard::Move::RIGHT;
        bboard::FillDestPos(s, m, des);
        bboard::ResolveDependencies(s, des, dependency, chain);

        REQUIRE_ROOTS(chain, 1);
    }
    SECTION("Resolve 0->1 and 2->3 Dependency")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 8, 8);
        s->PutAgent(3, 9, 8);

        m[0] = m[1] = m[2] = bboard::Move::RIGHT;
        bboard::FillDestPos(s, m, des);
        bboard::ResolveDependencies(s, des, dependency, chain);

        REQUIRE_ROOTS(chain, 1, 3);
    }
    SECTION("Resolve Complete Chain")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 2, 0);
        s->PutAgent(3, 3, 0);

        m[0] = m[1] = m[2] = m[3] = bboard::Move::RIGHT;
        bboard::FillDestPos(s, m, des);
        bboard::ResolveDependencies(s, des, dependency, chain);

        REQUIRE_ROOTS(chain, 3);
    }
    SECTION("Resolve Ouroboros")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 1, 1);
        s->PutAgent(3, 0, 1);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::DOWN;
        m[2] = bboard::Move::LEFT;
        m[3] = bboard::Move::UP;

        bboard::FillDestPos(s, m, des);
        int rootC = bboard::ResolveDependencies(s, des, dependency, chain);

        REQUIRE(chain[0] == -1);
        REQUIRE(rootC == 0);
    }
    SECTION("Handle Dead Agents as Roots")
    {
        s->PutAgent(0, 0, 0);
        s->PutAgent(1, 1, 0);
        s->PutAgent(2, 1, 1);
        s->PutAgent(3, 0, 1);

        m[0] = bboard::Move::RIGHT;
        m[1] = bboard::Move::DOWN;
        m[2] = bboard::Move::LEFT;
        m[3] = bboard::Move::UP;

        s->Kill(1);

        bboard::FillDestPos(s, m, des);
        bboard::ResolveDependencies(s, des, dependency, chain);

        REQUIRE_ROOTS(chain, 0, 1);
    }
    delete s;
}
