#include <iostream>
#include <random>
#include <stack>

#include "bboard.hpp"

namespace bboard
{

void State::PutAgent(int agentID, int x, int y)
{
    int b = Item::AGENT0 + agentID;
    board[y][x] = b;
    agentX[agentID] = x;
    agentY[agentID] = y;

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

    // note: the rest of the vals can remain 0
    agentX[a1] = agentX[a2] = BOARD_SIZE - 1;
    agentY[a2] = agentY[a3] = BOARD_SIZE - 1;

    agents[a1].x = agents[a2].x = BOARD_SIZE - 1;
    agents[a2].y = agents[a3].y = BOARD_SIZE - 1;
}

State* InitEmpty(int a0, int a1, int a2, int a3)
{
    State* result = new State();
    result->PutAgentsInCorners(a0, a1, a2, a3);
    return result;
}

State* InitState(int a0, int a1, int a2, int a3)
{
    State* result = new State();

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
    return result;
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

