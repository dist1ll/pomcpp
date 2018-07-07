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

    // tick flames
    // <--------->

    //tick bombs
    for(int i = 0; i < state->bombQueue.bombsOnBoard; i++)
    {
        state->bombQueue[i].timeLeft--;
    }

    //explode timed-out bombs
    int bombCount = state->bombQueue.bombsOnBoard;
    for(int i = 0; i < bombCount; i++)
    {
        if(state->bombQueue[0].timeLeft == 0)
        {
            Bomb& c = state->bombQueue[0];
            state->SpawnFlames(c.position.x, c.position.y, c.strength);
            state->PopBomb();
        }
        else
        {
            break;
        }

    }

    ///////////////////////
    //  Player Movement  //
    ///////////////////////

    Position destPos[AGENT_COUNT];
    FillDestPos(state, moves, destPos);
    FixSwitchMove(state, destPos);

    int dependency[AGENT_COUNT] = {-1, -1, -1, -1};
    int roots[AGENT_COUNT] = {-1, -1, -1, -1};

    // the amount of chain roots
    const int rootNumber = ResolveDependencies(state, destPos, dependency, roots);
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
        Move m = moves[i];

        if(state->agents[i].dead || m == Move::IDLE)
        {
            continue;
        }
        else if(m == Move::BOMB)
        {
            state->PlantBomb(i, state->agents[i].x, state->agents[i].y);
            continue;
        }
        int x = state->agents[i].x;
        int y = state->agents[i].y;

        Position desired = destPos[i];
        int itemOnDestination = state->board[desired.y][desired.x];

        //if ouroboros, the bomb will be covered by an agent
        if(ouroboros)
        {
            for(int j = 0; j < state->bombQueue.bombsOnBoard; j++)
            {
                if(state->bombQueue[j].position == desired)
                {
                    itemOnDestination = Item::BOMB;
                    break;
                }
            }
        }

        if(IsOutOfBounds(desired))
        {
            continue;
        }

        if(HasDPCollision(*state, destPos, i))
        {
            continue;
        }

        //
        // All checks passed - you can try a move now
        //


        // Collect those sweet power-ups
        if(itemOnDestination == Item::EXTRABOMB)
        {
            state->agents[i].maxBombCount++;
            itemOnDestination = 0; // consume the power-up
        }
        else if(itemOnDestination == Item::INCRRANGE)
        {
            state->agents[i].bombStrength++;
            itemOnDestination = 0; // consume the power-up
        }
        else if(itemOnDestination == Item::KICK)
        {
            state->agents[i].canKick = true;
            itemOnDestination = 0; // consume the power-up
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
