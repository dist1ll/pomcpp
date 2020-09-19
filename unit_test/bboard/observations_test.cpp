#include <iostream>

#include "catch.hpp"
#include "bboard.hpp"

using namespace bboard;

void REQUIRE_CORRECT_FOG(const State& s, uint agentID, const Board& b, int viewRange)
{
    bool foundIncorrectCell = false;
    for(int y = 0; y < BOARD_SIZE; y++)
    {
        for(int x = 0; x < BOARD_SIZE; x++)
        {
            bool isFog = b.items[y][x] == Item::FOG;
            bool isInViewRange = InViewRange(s.agents[agentID].GetPos(), x, y, viewRange);

            if(isFog == isInViewRange)
            {
                foundIncorrectCell = true;
                break;
            }
        }

        if(foundIncorrectCell)
            break;
    }
    REQUIRE(foundIncorrectCell == false);
}

TEST_CASE("View Range", "[observation]")
{
    // initialize some state
    State s;
    s.Init(GameMode::FreeForAll, 4, true);

    SECTION("No fog in state")
    {
        bool foundFog = false;
        for(int y = 0; y < BOARD_SIZE; y++)
        {
            for(int x = 0; x < BOARD_SIZE; x++)
            {
                if(s.items[y][x] == Item::FOG)
                {
                    foundFog = true;
                    break;
                }
            }

            if(foundFog)
                break;
        }
        REQUIRE(foundFog == false);
    }

    SECTION("Fog in observation 0")
    {
        ObservationParameters params;
        params.agentPartialMapView = true;
        params.agentViewSize = 3;

        Observation obs;
        Observation::Get(s, 0, params, obs);

        REQUIRE_CORRECT_FOG(s, 0, obs, params.agentViewSize);
    }

    SECTION("Fog in observation 2")
    {
        ObservationParameters params;
        params.agentPartialMapView = true;
        params.agentViewSize = 5;

        Observation obs;
        Observation::Get(s, 2, params, obs);

        REQUIRE_CORRECT_FOG(s, 2, obs, params.agentViewSize);
    }
}

TEST_CASE("Round trip", "[observation]")
{
    // initialize state
    State s;
    s.Init(GameMode::FreeForAll, 1234, true);

    // restrict view
    ObservationParameters params;
    params.agentPartialMapView = true;
    params.agentViewSize = 4;

    // get observation
    Observation obs;
    Observation::Get(s, 2, params, obs);

    // check fog
    REQUIRE_CORRECT_FOG(s, 2, obs, params.agentViewSize);

    // convert to state
    State s2;
    obs.ToState(s2, GameMode::FreeForAll);

    // fog is still there
    REQUIRE_CORRECT_FOG(s, 2, s2, params.agentViewSize);
    REQUIRE(s.aliveAgents == s2.aliveAgents);
    REQUIRE(s.agents[2].x == s2.agents[2].x);
    REQUIRE(s.agents[2].y == s2.agents[2].y);
    REQUIRE(s.agents[2].team == s2.agents[2].team);
    REQUIRE(s2.agents[2].ignore == false);
}

TEST_CASE("Planning Step", "[observation]")
{
    ObservationParameters params;
    params.agentPartialMapView = true;
    params.agentViewSize = 4;

    State s, s2;
    Observation obs;

    s.Init(GameMode::FreeForAll, 1234, true);
    Observation::Get(s, 0, params, obs);
    obs.ToState(s2, GameMode::FreeForAll);

    Move m[4];
    m[0] = m[1] = m[2] = m[3] = Move::IDLE;

    AgentInfo& ownAgent = s2.agents[0];

    SECTION("Step")
    {
        // just check whether we can do a step
        bboard::Step(&s2, m);
    }
    SECTION("Bombs explode")
    {
        REQUIRE(!ownAgent.dead);
        m[0] = Move::BOMB;
        bboard::Step(&s2, m);
        m[0] = Move::IDLE;

        for(int _ = 0; _ < bboard::BOMB_LIFETIME; _++)
        {
            bboard::Step(&s2, m);
        }

        REQUIRE(ownAgent.dead);
    }
}
