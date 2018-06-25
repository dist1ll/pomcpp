#include <iostream>

#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard
{

Position DesiredPosition(int x, int y, Move move)
{
    Position p;
    p.x = x;
    p.y = y;
    if(move == Move::UP)
    {
        p.y -= 1;
    }
    else if(move == Move::DOWN)
    {
        p.y += 1;
    }
    else if(move == Move::LEFT)
    {
        p.x -= 1;
    }
    else if(move == Move::RIGHT)
    {
        p.x += 1;
    }
    return p;
}
void FillDestPos(State* s, Move m[AGENT_COUNT], Position p[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        p[i] = DesiredPosition(s->agentX[i], s->agentY[i], m[i]);
    }
}

void FixSwitchMove(State* s, Position d[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        for(int j = i; j < AGENT_COUNT; j++)
        {
            if(d[i].x == s->agentX[j] && d[i].y == s->agentY[j] &&
                    d[j].x == s->agentX[i] && d[j].y == s->agentY[i])
            {
                d[i].x = s->agentX[i];
                d[i].y = s->agentY[i];
                d[j].x = s->agentX[j];
                d[j].y = s->agentY[j];
            }
        }
    }
}

int ResolveDependencies(State* s, Position des[AGENT_COUNT],
                        int dependency[AGENT_COUNT], int chain[AGENT_COUNT])
{
    int rootCount = 0;
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        // dead agents are handled as roots
        if(s->dead[i])
        {
            chain[rootCount] = i;
            rootCount++;
            continue;
        }

        bool isChainRoot = true;
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            if(i == j || s->dead[j]) continue;

            if(des[i].x == s->agentX[j] && des[i].y == s->agentY[j])
            {
                dependency[j] = i;
                isChainRoot = false;
                break;
            }
        }
        if(isChainRoot)
        {
            chain[rootCount] = i;
            rootCount++;
        }
    }
    return rootCount;
}

void PrintDependency(int dependency[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        if(dependency[i] == -1)
        {
            std::cout << "[" << i << " <- ]";
        }
        else
        {
            std::cout << "[" << i << " <- " << dependency[i] << "]";
        }
        std::cout << std::endl;
    }
}

void PrintDependencyChain(int dependency[AGENT_COUNT], int chain[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        if(chain[i] == -1) continue;

        std::cout << chain[i];
        int k = dependency[chain[i]];

        while(k != -1)
        {
            std::cout << " <- " << k;
            k = dependency[k];
        }
        std::cout << std::endl;
    }
}


}
