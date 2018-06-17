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
    int chainRoots[AGENT_COUNT] = {-1, -1, -1, -1};
    // the amount of chain roots
    int rootNumber = ResolveDependencies(state, destPos, dependency, chainRoots);

    // iterates 4 times but the index i jumps around the dependencies
    for(int i = 0; i < AGENT_COUNT; i++)
    {
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
