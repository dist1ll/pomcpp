#include <iostream>

#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard
{


void Step(State* state, Move* moves)
{
    ///////////////////////
    // Flames, Explosion //
    ///////////////////////

    util::TickFlames(*state);
    util::TickBombs(*state);

    ///////////////////////
    //  Player Movement  //
    ///////////////////////

    Position destPos[AGENT_COUNT];
    util::FillDestPos(state, moves, destPos);
    util::FixSwitchMove(state, destPos);

    int dependency[AGENT_COUNT] = {-1, -1, -1, -1};
    int roots[AGENT_COUNT] = {-1, -1, -1, -1};

    // the amount of chain roots
    const int rootNumber = util::ResolveDependencies(state, destPos, dependency, roots);
    const bool ouroboros = rootNumber == 0; // ouroboros formation?

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
        const Move m = moves[i];

        if(state->agents[i].dead || m == Move::IDLE)
        {
            continue;
        }
        else if(m == Move::BOMB)
        {
            state->PlantBomb(state->agents[i].x, state->agents[i].y, i);
            continue;
        }


        int x = state->agents[i].x;
        int y = state->agents[i].y;

        Position desired = destPos[i];

        if(util::IsOutOfBounds(desired))
        {
            continue;
        }

        int itemOnDestination = state->board[desired.y][desired.x];

        //if ouroboros, the bomb will be covered by an agent
        if(ouroboros)
        {
            for(int j = 0; j < state->bombs.count; j++)
            {
                if(BMB_POS_X(state->bombs[j]) == desired.x
                        && BMB_POS_Y(state->bombs[j]) == desired.y)
                {
                    itemOnDestination = Item::BOMB;
                    break;
                }
            }
        }

        if(IS_FLAME(itemOnDestination))
        {
            state->Kill(i);
            if(state->board[y][x] == Item::AGENT0 + i)
            {
                if(state->HasBomb(x, y))
                {
                    state->board[y][x] = Item::BOMB;
                }
                else
                {
                    state->board[y][x] = Item::PASSAGE;
                }

            }
            continue;
        }
        if(util::HasDPCollision(*state, destPos, i))
        {
            continue;
        }

        //
        // All checks passed - you can try a move now
        //


        // Collect those sweet power-ups
        if(IS_POWERUP(itemOnDestination))
        {
            util::ConsumePowerup(*state, i, itemOnDestination);
            itemOnDestination = 0;
        }

        // execute move if the destination is free
        // (in the rare case of ouroboros, make the move even
        // if an agent occupies the spot)
        if(itemOnDestination == 0 ||
                (ouroboros && itemOnDestination >= Item::AGENT0))
        {
            // only override the position I came from if it has not been
            // overridden by a different agent that already took this spot
            if(state->board[y][x] == Item::AGENT0 + i)
            {
                if(state->HasBomb(x, y))
                {
                    state->board[y][x] = Item::BOMB;
                }
                else
                {
                    state->board[y][x] = 0;
                }

            }
            state->board[desired.y][desired.x] = Item::AGENT0 + i;
            state->agents[i].x = desired.x;
            state->agents[i].y = desired.y;
        }

    }

}

}
