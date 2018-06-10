#include <iostream>
#include <random>
#include <stack>

#include "bboard.hpp"

namespace bboard
{


State* InitState(int a0, int a1, int a2, int a3)
{
    State* result = new State();
    int b = Item::AGENT0; // agent no. offset

    // Randomly put obstacles
    std::mt19937_64 rng(0x1337);
    std::uniform_int_distribution<int> intDist(0,6);
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int  j = 0; j < BOARD_SIZE; j++)
        {
            result->board[i][j] = intDist(rng) < 5 ? 0 : 2;
        }
    }

    // Put agents
    result->board[0][0] = b + a0;
    result->board[0][BOARD_SIZE - 1] = b + a1;
    result->board[BOARD_SIZE - 1][BOARD_SIZE - 1] = b + a2;
    result->board[BOARD_SIZE - 1][0] = b + a3;

    // note: the rest of the vals can remain 0
    result->agentX[a1] = result->agentX[a2] = BOARD_SIZE - 1;
    result->agentY[a2] = result->agentY[a3] = BOARD_SIZE - 1;

    return result;
}

void Step(State* state, Move* moves)
{

    Position destPos[AGENT_COUNT];
    int dependency[AGENT_COUNT][2];

    for(int i = 0; i < AGENT_COUNT; i++)
    {
        destPos[i] = DesiredPosition(state->agentX[i], state->agentY[i], moves[i]);
    }

    for(int i = 0; i < AGENT_COUNT; i++)
    {
        Move m = moves[i];

        if(state->dead[i] || m == Move::IDLE)
        {
            continue;
        }

        int x = state->agentX[i];
        int y = state->agentY[i];
        dependency[i][0] = i;
        dependency[i][1] = -1;

        Position desired = destPos[i];

        // check out of bounds
        if(desired.x < 0 || desired.y < 0 ||
                desired.x >= BOARD_SIZE ||
                desired.y >= BOARD_SIZE)
        {
            continue;
        }

        // check for destination position collision
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            if(j == i) continue;

            if(destPos[j] == desired)
            {
                continue;
            }
        }

        // check for immediate agent collision
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            if(j == i) continue;

            if(state->agentX[j] == desired.x &&
                    state->agentY[j] == desired.y)
            {
                // don't execute a swap movement
                if(destPos[j].x == x && destPos[j].y == y)
                {
                    goto end_this_agent_move;
                }
                else
                {
                    dependency[i][1] = j;
                    goto end_this_agent_move;
                }
            }
        }

        // execute move if no obstacle
        if(state->board[desired.y][desired.x] == 0)
        {
            state->board[y][x] = 0;
            state->board[desired.y][desired.x] = Item::AGENT0 + i;
            state->agentX[i] = desired.x;
            state->agentY[i] = desired.y;
        }

end_this_agent_move:
        ;
    }
}

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

void PrintState(State* state)
{
    std::string result = "";

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            int item = state->board[i][j];
            result += PrintItem(item);
            if(j == BOARD_SIZE - 1)
            {
                result += "\n";
            }
        }
    }
    std::cout << result;
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
            return "[\u25A0]";
        case Item::BOMB:
            return " \u2B24 ";
        case Item::FLAMES:
            return " \u2B24 ";
    }
    //agent number
    if(item >= Item::AGENT0)
    {
        return " "  +  std::to_string(item - 10) + " ";
    }
    else
    {
        return std::to_string(item);
    }
}

}

