#include <iostream>
#include "bboard.hpp"

namespace bboard
{


State* InitState(Position* p)
{
    //TODO: init state for given agent positions
}

void Step(State* s, Move* m)
{
    //TODO: calculate step transition
}

void PrintState(State* s)
{
    std::string result = "";

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            result += (std::to_string(s->board[i][j]));
            result += " ";
            if(j == BOARD_SIZE - 1)
            {
                result += "\n";
            }
        }
    }
    std::cout << result;
}

}

