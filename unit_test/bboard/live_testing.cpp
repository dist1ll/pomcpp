#include "catch.hpp"
#include "colors.hpp"
#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"

#include "testing_utilities.hpp"

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
            std::array<SimpleAgent, 4> r = CreateAgents(rng);

            // create an environment
            bboard::Environment e;

            // initializes the game/board/agents
            e.MakeGame({&r[0], &r[1], &r[2], &r[3]}, bboard::GameMode::FreeForAll, (int)rng());

            // starts the game with the specified params
            e.RunGame(800, false, false);

            REQUIRE_CORRECT_RESULT_FFA(e);
        }
        std::cout << "done." << std::endl;
    }
    SECTION("Teams")
    {
        std::cout << "Mode \"TwoTeams\"... " << std::flush;
        for(int i = 0; i < numGames; i++)
        {
            std::array<SimpleAgent, 4> r = CreateAgents(rng);

            // create an environment
            bboard::Environment e;

            // initializes the game/board/agents
            e.MakeGame({&r[0], &r[1], &r[2], &r[3]}, bboard::GameMode::TwoTeams, (int)rng());

            // starts the game with the specified params
            e.RunGame(800, false, false);

            REQUIRE_CORRECT_RESULT_TEAMS(e);
        }
        std::cout << "done." << std::endl;
    }
}

TEST_CASE("Win Rate Stats", "[stats info]")
{
    int numGames = 2000;

    int seed = 42;
    std::mt19937 rng(seed);

    std::array<SimpleAgent, 4> simpleAgents = CreateAgents(rng);
    std::array<SimpleUnbiasedAgent, 4> simpleUnbiasedAgents = CreateUnbiasedAgents(rng);

    for (auto i : {0, 1})
    {
        std::array<bboard::Agent*, 4> agents;
        std::string agent_type;
        switch (i)
        {
        case 0:
            agent_type += "SimpleAgent";
            agents = {&simpleAgents[0], &simpleAgents[1], &simpleAgents[2], &simpleAgents[3]};
            break;

        case 1:
            agent_type += "SimpleUnbiasedAgent";
            agents = {&simpleUnbiasedAgents[0], &simpleUnbiasedAgents[1], &simpleUnbiasedAgents[2], &simpleUnbiasedAgents[3]};
            break;
        
        default:
            throw "Internal test error.";
            break;
        }

        int wins[bboard::AGENT_COUNT];
        std::fill_n(wins, bboard::AGENT_COUNT, 0);
        int draws = 0;
        int notDone = 0;
        double averageSteps = 0;

        for(bool useRandomAgentPos : {false, true})
        {
            std::string name = "Free For All (" + agent_type + ") - Random Start Positions = " + std::to_string(useRandomAgentPos);
            SECTION(name)
            {
                std::string tst = "Collecting stats for " + std::to_string(numGames) + " live games (" + agent_type + ")\n";
                std::cout << std::endl
                        << FGRN(tst);

                std::cout << name << std::endl;
                for(int i = 0; i < numGames; i++)
                {
                    bboard::Environment e;
                    long randomAgentPositionSeed = useRandomAgentPos ? rng() : -1;
                    e.MakeGame(agents, bboard::GameMode::FreeForAll, rng(), randomAgentPositionSeed);
                    e.RunGame(800, false, false);

                    bboard::State finalState = e.GetState();

                    if (i == 0) {
                        averageSteps = finalState.timeStep;
                    }
                    else {
                        averageSteps = (i * averageSteps + finalState.timeStep) / (i + 1);
                    }

                    int winner = finalState.winningAgent;
                    if (winner != -1) {
                        wins[winner]++;
                    }
                    notDone += finalState.finished ? 0 : 1;
                    draws += finalState.isDraw ? 1 : 0;
                }

                std::cout << "Episodes: " << numGames << std::endl;
                std::cout << "Average steps: " << averageSteps << std::endl;
                std::cout << "Wins:" << std::endl;
                for (int i = 0; i < bboard::AGENT_COUNT; i++) {
                    std::cout << "> Agent " << i << ": " << wins[i] << " (" << (float)wins[i] / numGames * 100 << "%)" << std::endl;
                }
                std::cout << "Draws: " << draws <<  " (" << (float)draws / numGames * 100 << "%)" << std::endl;
                std::cout << "Not done: " << notDone <<  " (" << (float)notDone / numGames * 100 << "%)" << std::endl;
            }
        }
    }
}
