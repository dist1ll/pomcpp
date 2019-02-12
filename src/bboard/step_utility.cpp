#include <iostream>

#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard::util
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

Position DesiredPosition(const Bomb b)
{
    return DesiredPosition(BMB_POS_X(b), BMB_POS_Y(b), Move(BMB_DIR(b)));
}

void FillPositions(State* s, Position p[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        p[i] = {s->agents[i].x, s->agents[i].y};
    }
}

void FillDestPos(State* s, Move m[AGENT_COUNT], Position p[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        p[i] = DesiredPosition(s->agents[i].x, s->agents[i].y, m[i]);
    }
}

void FixSwitchMove(State* s, Position d[AGENT_COUNT])
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        for(int j = i; j < AGENT_COUNT; j++)
        {
            if(d[i].x == s->agents[j].x && d[i].y == s->agents[j].y &&
                    d[j].x == s->agents[i].x && d[j].y == s->agents[i].y)
            {
                d[i].x = s->agents[i].x;
                d[i].y = s->agents[i].y;
                d[j].x = s->agents[j].x;
                d[j].y = s->agents[j].y;
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
        if(s->agents[i].dead)
        {
            chain[rootCount] = i;
            rootCount++;
            continue;
        }

        bool isChainRoot = true;
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            if(i == j || s->agents[j].dead) continue;

            if(des[i].x == s->agents[j].x && des[i].y == s->agents[j].y)
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


void TickFlames(State& state)
{
    for(int i = 0; i < state.flames.count; i++)
    {
        state.flames[i].timeLeft--;
    }
    int flameCount = state.flames.count;
    for(int i = 0; i < flameCount; i++)
    {
        if(state.flames[0].timeLeft == 0)
        {
            state.PopFlame();
        }
    }
}

void TickBombs(State& state)
{
    for(int i = 0; i < state.bombs.count; i++)
    {
        ReduceBombTimer(state.bombs[i]);
    }

    //explode timed-out bombs
    int bombCount = state.bombs.count;
    for(int i = 0; i < bombCount && state.bombs.count > 0; i++)
    {
        if(BMB_TIME(state.bombs[0]) == 0)
        {
            state.ExplodeTopBomb();
        }
        else
        {
            break;
        }

    }
}

void ConsumePowerup(State& state, int agentID, int powerUp)
{
    if(powerUp == Item::EXTRABOMB)
    {
        state.agents[agentID].maxBombCount++;
    }
    else if(powerUp == Item::INCRRANGE)
    {
        state.agents[agentID].bombStrength++;
    }
    else if(powerUp == Item::KICK)
    {
        state.agents[agentID].canKick = true;
    }

}

bool HasDPCollision(const State& state, Position dp[AGENT_COUNT], int agentID)
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        if(agentID == i || state.agents[i].dead) continue;
        if(dp[agentID] == dp[i])
        {
            // a destination position conflict will never
            // result in a valid move
            return true;
        }
    }
    return false;
}

bool HasBombCollision(const State& state, const Bomb& b, int index)
{
    Position bmbTarget = util::DesiredPosition(b);

    for(int i = index; i < state.bombs.count; i++)
    {
        Position target = util::DesiredPosition(state.bombs[i]);

        if(b != state.bombs[i] && target == bmbTarget)
        {
            return true;
        }
    }
    return false;
}

void ResolveBombCollision(State& state, Bomb& b, int index)
{
    Bomb collidees[4]; //more than 4 bombs cannot collide
    Position bmbTarget = util::DesiredPosition(b);

    for(int i = index; i < state.bombs.count; i++)
    {
        Position target = util::DesiredPosition(state.bombs[i]);

        if(b != state.bombs[i] && target == bmbTarget)
        {
            SetBombDirection(state.bombs[i], Direction::IDLE);
        }
    }

    SetBombDirection(b, Direction::IDLE);
}

void ResetBombFlags(State& state)
{
    for(int i = 0; i < state.bombs.count; i++)
    {
        SetBombMovedFlag(state.bombs[i], false);
    }
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
