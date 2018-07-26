#include <thread>
#include <chrono>
#include <functional>

#include "bboard.hpp"

namespace bboard
{

void Pause(bool timeBased)
{
    if(!timeBased)
        std::getchar();
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
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

    state->PutItem(1, 4, bboard::Item::INCRRANGE);
    state->PutItem(6, 4, bboard::Item::KICK);
    state->PutItem(7, 6, bboard::Item::EXTRABOMB);

    state->PutAgentsInCorners(0, 1, 2, 3);

    SetAgents(a);
    hasStarted = true;
}

void Environment::StartGame(int timeSteps, bool render, bool stepByStep)
{
    state->timeStep = 0;
    Print();
    if(stepByStep)
        std::getchar();

    while(!this->IsDone() && state->timeStep < timeSteps)
    {
        this->Step();
        if(render)
        {
            Print();

            if(listener)
                listener(*state.get());

            Pause(!stepByStep);
        }
    }
    PrintGameResult(*this);
}

void Environment::Step()
{
    if(!hasStarted || finished)
    {
        return;
    }

    Move m[AGENT_COUNT];
    for(uint i = 0; i < AGENT_COUNT; i++)
    {
        m[i] = agents[i]->act(state.get());
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
    if(clear)
        std::cout << "\033c"; // clear console on linux
    PrintState(state.get());
}

State& Environment::GetState() const
{
    return *state.get();
}

void Environment::SetAgents(std::array<Agent*, AGENT_COUNT> agents)
{
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

void Environment::SetStepListener(const std::function<void(const State&)>& f)
{
    listener = f;
}

}
