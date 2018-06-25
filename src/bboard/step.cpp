#include <iostream>

#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard
{


void Step(State* state, Move* moves)
{

    Position destPos[AGENT_COUNT];

    FillDestPos(state, moves, destPos);

    FixSwitchMove(state, destPos); // fixes the switching

    int dependency[AGENT_COUNT] = {-1, -1, -1, -1};
    int roots[AGENT_COUNT] = {-1, -1, -1, -1};
    // the amount of chain roots
    int rootNumber = ResolveDependencies(state, destPos, dependency, roots);

    int rootIdx = 0;
    int i = rootNumber == 0 ? 0 : roots[0]; // no roots -> start from 0
    // iterates 4 times but the index i jumps around the dependencies
    for(int _ = 0; _ < AGENT_COUNT; _++, i = dependency[i])
    {
        if(i == -1)
        {
            rootIdx++;
            i = roots[rootIdx];
        }
        Move m = moves[i];

        if(state->dead[i] || m == Move::IDLE)
        {
            continue;
        }

        int x = state->agentX[i];
        int y = state->agentY[i];

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
            if(j == i || state->dead[j]) continue;

            if(destPos[j] == desired)
            {
                // a destination position conflict will never
                // result in a valid move
                goto end_this_agent_move;
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

}
