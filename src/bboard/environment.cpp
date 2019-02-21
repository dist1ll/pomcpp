#include <thread>
#include <chrono>
#include <functional>

#include "bboard.hpp"

namespace bboard
{

void Pause(bool timeBased)
{
    if(!timeBased)
    {
        std::cin.get();
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void PrintGameResult(Environment& env)
{
    std::cout << std::endl;

    if(env.IsDone())
    {
        if(env.IsDraw())
        {
            std::cout << "Draw! All agents are dead"
                      << std::endl;
        }
        else
        {
            std::cout << "Finished! The winner is Agent "
                      << env.GetWinner() << std::endl;
        }

    }
    else
    {
        std::cout << "Draw! Max timesteps reached "
                  << std::endl;
    }
}

Environment::Environment()
{
    state = std::make_unique<State>();
}

void Environment::MakeGame(std::array<Agent*, AGENT_COUNT> a)
{
    bboard::InitState(state.get(), 0, 1, 2, 3);

    state->PutAgentsInCorners(0, 1, 2, 3);

    SetAgents(a);
    hasStarted = true;
}

void Environment::StartGame(int timeSteps, bool render, bool stepByStep)
{
    state->timeStep = 0;
    while(!this->IsDone() && state->timeStep < timeSteps)
    {

        if(render)
        {
            Print();

            if(listener)
                listener(*this);

            if(stepByStep)
                Pause(false);
        }
        this->Step(true);
    }
    Print();
    PrintGameResult(*this);
}

void ProxyAct(Move& writeBack, Agent& agent, State& state)
{
    writeBack = agent.act(&state);
}

void CollectMovesAsync(Move m[AGENT_COUNT], Environment& e)
{
    std::thread threads[AGENT_COUNT];
    for(uint i = 0; i < AGENT_COUNT; i++)
    {
        if(!e.GetState().agents[i].dead)
        {
            threads[i] = std::thread(ProxyAct,
                                     std::ref(m[i]),
                                     std::ref(*e.GetAgent(i)),
                                     std::ref(e.GetState()));
        }
    }
    Pause(true); //competitive pause
    for(uint i = 0; i < AGENT_COUNT; i++)
    {
        if(!e.GetState().agents[i].dead)
        {
            threads[i].join();
        }
    }
}

Move Environment::GetLastMove(int agentID)
{
    return lastMoves[agentID];
}

void Environment::Step(bool competitiveTimeLimit)
{
    if(!hasStarted || finished)
    {
        return;
    }

    Move m[AGENT_COUNT];


    if(competitiveTimeLimit)
    {
        CollectMovesAsync(m, *this);
    }
    else
    {
        for(uint i = 0; i < AGENT_COUNT; i++)
        {
            if(!state->agents[i].dead)
            {
                m[i] = agents[i]->act(state.get());
                lastMoves[i] = m[i];
            }
        }
    }

    bboard::Step(state.get(), m);
    state->timeStep++;

    if(state->aliveAgents == 1)
    {
        finished = true;
        for(int i = 0; i < AGENT_COUNT; i++)
        {
            if(!state->agents[i].dead)
            {
                agentWon = i;
                // teamwon = team of agent
            }
        }
    }
    if(state->aliveAgents == 0)
    {
        finished = true;
        isDraw = true;
    }
}

void Environment::Print(bool clear)
{
    PrintState(state.get(), true);
}

State& Environment::GetState() const
{
    return *state.get();
}

Agent* Environment::GetAgent(uint agentID) const
{
    return agents[agentID];
}

void Environment::SetAgents(std::array<Agent*, AGENT_COUNT> agents)
{
    for(uint i = 0; i < AGENT_COUNT; i++)
    {
        agents[i]->id = int(i);
    }
    this->agents = agents;
}

bool Environment::IsDone()
{
    return finished;
}

bool Environment::IsDraw()
{
    return isDraw;
}

int Environment::GetWinner()
{
    return agentWon;
}

void Environment::SetStepListener(const std::function<void(const Environment&)>& f)
{
    listener = f;
}

}
