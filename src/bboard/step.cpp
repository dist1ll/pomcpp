#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard
{


void Step(State* state, Move* moves)
{

    Position destPos[AGENT_COUNT];

    //dependency[i] = j <==> Agent i wants j's spot,
    //and j wants to move somewhere else
    int dependency[AGENT_COUNT];

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

        dependency[i] = -1;

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
                // a destination position conflict will never
                // result in a valid move
                goto end_this_agent_move;
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
                    dependency[i] = j;
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

}
