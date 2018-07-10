#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <stack>

#include "bboard.hpp"
#include "colors.hpp"

namespace bboard
{

/////////////////////////
// Auxiliary Functions //
/////////////////////////

inline bool SpawnFlameItem(State& s, int x, int y)
{
    if(s.board[y][x] >= Item::AGENT0)
    {
        s.Kill(s.board[y][x] - Item::AGENT0);
    }
    if(s.board[y][x] == Item::BOMB)
    {
        //TODO: Chain reaction
    }


    if(s.board[y][x] != Item::RIGID)
    {
        s.board[y][x] = Item::FLAMES;
        return true;
    }
    else
    {
        return false;
    }
}

void PrintGameResult(Environment& env)
{
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

///////////////////
// State Methods //
///////////////////

void State::PlantBomb(int id, int x, int y)
{
    if(agents[id].bombCount >= agents[id].maxBombCount)
    {
        return;
    }

    Bomb* b = &bombQueue.NextPos();
    b->id = id;
    b->position.x = x;
    b->position.y = y;
    b->strength = agents[id].bombStrength;
    b->velocity = Direction::IDLE;
    b->timeLeft = BOMB_LIFETIME;

    agents[id].bombCount++;
    bombQueue.count++;
}

void State::PopBomb()
{
    agents[bombQueue[0].id].bombCount--;
    bombQueue.PopElem();
}

void State::PopFlame()
{
    Flame& f = flames[0];

    // remove the flame from the board

    flames.PopElem();
}

void State::SpawnFlame(int x, int y, int strength)
{
    Flame& f = flames.NextPos();
    f.position.x = x;
    f.position.y = y;
    f.strength = strength;
    f.timeLeft = FLAME_LIFETIME;

    flames.count++;

    // right
    for(int i = 0; i <= strength; i++)
    {
        if(x + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x + i, y))
        {
            break;
        }
    }

    // left
    for(int i = 0; i <= strength; i++)
    {
        if(x - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x - i, y))
        {
            break;
        }
    }

    // top
    for(int i = 0; i <= strength; i++)
    {
        if(y + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x, y + i))
        {
            break;
        }
    }

    // bottom
    for(int i = 0; i <= strength; i++)
    {
        if(y - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x, y - i))
        {
            break;
        }
    }
}

bool State::HasBomb(int x, int y)
{
    for(int i = 0; i < bombQueue.count; i++)
    {
        if(bombQueue[i].position.x == x && bombQueue[i].position.y == y)
        {
            return true;
        }
    }
    return false;
}

void State::PutAgent(int agentID, int x, int y)
{
    int b = Item::AGENT0 + agentID;
    board[y][x] = b;

    agents[agentID].x = x;
    agents[agentID].y = y;
}

void State::PutAgentsInCorners(int a0, int a1, int a2, int a3)
{
    int b = Item::AGENT0;

    board[0][0] = b + a0;
    board[0][BOARD_SIZE - 1] = b + a1;
    board[BOARD_SIZE - 1][BOARD_SIZE - 1] = b + a2;
    board[BOARD_SIZE - 1][0] = b + a3;

    agents[a1].x = agents[a2].x = BOARD_SIZE - 1;
    agents[a2].y = agents[a3].y = BOARD_SIZE - 1;
}

/////////////////////////
// Environment Methods //
/////////////////////////

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

void Environment::StartGame(int timeSteps, bool render)
{
    int time = 0;
    while(!this->IsDone() && time < timeSteps)
    {
        this->Step();

        if(render)
        {
            std::cout << "\033c"; // clear console on linux
            PrintState(state.get());
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
        time++;
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

//////////////////////
// bboard namespace //
//////////////////////

void InitState(State* result, int a0, int a1, int a2, int a3)
{
    // Randomly put obstacles
    std::mt19937_64 rng(0x1337);
    std::uniform_int_distribution<int> intDist(0,6);
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int  j = 0; j < BOARD_SIZE; j++)
        {
            int tmp = intDist(rng);
            result->board[i][j] = tmp > 2 ? 0 : tmp;
        }
    }

    result->PutAgentsInCorners(a0, a1, a2, a3);
}


void StartGame(State* state, Agent* agents[AGENT_COUNT], int timeSteps)
{
    Move moves[4];

    for(int i = 0; i < timeSteps; i++)
    {
        std::cout << "\033c"; // clear console on linux
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            moves[j] = agents[j]->act(state);
        }

        Step(state, moves);
        PrintState(state);

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void PrintState(State* state)
{
    std::string result = "";

    for(int y = 0; y < BOARD_SIZE; y++)
    {
        for(int x = 0; x < BOARD_SIZE; x++)
        {
            int item = state->board[y][x];
            result += PrintItem(item);
            if(x == BOARD_SIZE - 1)
            {
                result += "\n";
            }
        }
    }
    std::cout << result << std::endl << std::endl;

    //Agent info
    for(int _ = 0; _ < AGENT_COUNT; _++)
    {
        int i = _;
        std::printf("A%d: %s %d  %s %d  %s %d\n",
                    i,
                    PrintItem(Item::EXTRABOMB).c_str(),state->agents[i].maxBombCount,
                    PrintItem(Item::INCRRANGE).c_str(),state->agents[i].bombStrength,
                    PrintItem(Item::KICK).c_str(),state->agents[i].canKick);
    }
    std::cout << "\nBomb queue:  [  ";
    for(int i = 0; i < state->bombQueue.count; i++)
    {
        std::cout << state->bombQueue[i].id << "  ";
    }
    std::cout << "]\nFlame queue: [  ";
    for(int i = 0; i < state->flames.count; i++)
    {
        std::cout << state->flames[i].timeLeft << "  ";
    }
    std::cout << "]" << std::endl;
}

std::string PrintItem(int item)
{
    switch(item)
    {
        case Item::PASSAGE:
            return "   ";
        case Item::RIGID:
            return "[X]";
        case Item::WOOD:
            return FBLU("[\u25A0]");
        case Item::BOMB:
            return " \u25CF ";
        case Item::FLAMES:
            return FYEL(" \U0000263C ");
        case Item::EXTRABOMB:
            return " \u24B7 ";
        case Item::INCRRANGE:
            return " \u24C7 ";
        case Item::KICK:
            return " \u24C0 ";
    }
    //agent number
    if(item >= Item::AGENT0)
    {
        return " "  +  std::to_string(item - 10) + " ";
    }
    else
    {
        return "[?]";
    }
}

}

