#include "catch.hpp"
#include "colors.hpp"
#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"

using namespace agents;

template <int AGENT>
void PrintAgentInfo(const bboard::Environment& env)
{
    SimpleAgent& a = *static_cast<SimpleAgent*>(env.GetAgent(AGENT));
    std::cout << std::endl;
    // bboard::strategy::PrintMap(a.r);
    a.PrintDetailedInfo();
}

void REQUIRE_CORRECT_RESULT_FFA(bboard::Environment& env)
{
    if(env.IsDone())
    {
        // no winning team
        REQUIRE(env.GetWinningTeam() == 0);

        int winningAgent = env.GetWinningAgent();
        if(winningAgent != -1)
        {
            // either exactly one winner
            REQUIRE(env.GetState().agents[winningAgent].won == true);
            for(int i = 0; i < bboard::AGENT_COUNT; i++)
            {
                if(i == winningAgent) continue;

                REQUIRE(env.GetState().agents[i].won == false);
            }
        }
        else
        {
            // or no winners
            REQUIRE(env.IsDraw() == true);
        }
    }
    else
    {
        // at least two agents are still alive
        REQUIRE(env.GetState().aliveAgents >= 2);
    }
}

void REQUIRE_CORRECT_RESULT_TEAMS(bboard::Environment& env)
{
    if(env.IsDone())
    {
        // there is no single winning agent, we only consider teams
        REQUIRE(env.GetWinningAgent() == -1);

        int winningTeam = env.GetWinningTeam();
        if(winningTeam != 0)
        {
            for(int i = 0; i < bboard::AGENT_COUNT; i++)
            {
                bboard::AgentInfo info = env.GetState().agents[i];
                // an agent has won iff he is in the winning team
                REQUIRE(info.won == (info.team == winningTeam));
            }
        }
        else
        {
            // or no winners
            REQUIRE(env.IsDraw() == true);
        }
    }
    else
    {
        // at least two agents are still alive
        REQUIRE(env.GetState().aliveAgents >= 2);

        int team0 = 0;
        bool foundTwoTeams = false;
        // they have to be in two different teams
        for(int i = 0; i < bboard::AGENT_COUNT; i++)
        {
            bboard::AgentInfo info = env.GetState().agents[i];
            if(!info.dead && info.team != 0)
            {
                if (team0 == 0)
                {
                    team0 = info.team;
                    continue;
                }
                else if(team0 != info.team)
                {
                    foundTwoTeams = true;
                    break;
                }
            }
        }

        REQUIRE(foundTwoTeams);
    }
}

std::array<SimpleAgent, 4> createAgents(std::mt19937& rng)
{
    return {SimpleAgent(rng()), SimpleAgent(rng()), SimpleAgent(rng()), SimpleAgent(rng())};
}

TEST_CASE("Test Simple Agent", "[live testing]")
{
    int numGames = 100;

    int seed = 42;
    std::mt19937 rng(seed);

    std::string tst = "Testing " + std::to_string(numGames) + " live games\n";
    std::cout << std::endl
              << FGRN(tst);

    SECTION("Free For All")
    {
        std::cout << "Mode \"FreeForAll\"... " << std::flush;
        for(int i = 0; i < numGames; i++)
        {
            std::array<SimpleAgent, 4> r = createAgents(rng);

            // create an environment
            bboard::Environment e;

            // initializes the game/board/agents
            e.MakeGame({&r[0], &r[1], &r[2], &r[3]}, bboard::GameMode::FreeForAll, (int)rng());

            // starts the game with the specified params
            e.RunGame(800, true, false);

            REQUIRE_CORRECT_RESULT_FFA(e);
        }
        std::cout << "done." << std::endl;
    }
    SECTION("Teams")
    {
        std::cout << "Mode \"TwoTeams\"... " << std::flush;
        for(int i = 0; i < numGames; i++)
        {
            std::array<SimpleAgent, 4> r = createAgents(rng);

            // create an environment
            bboard::Environment e;

            // initializes the game/board/agents
            e.MakeGame({&r[0], &r[1], &r[2], &r[3]}, bboard::GameMode::TwoTeams, (int)rng());

            // starts the game with the specified params
            e.RunGame(800, true, false);

            REQUIRE_CORRECT_RESULT_TEAMS(e);
        }
        std::cout << "done." << std::endl;
    }
}
