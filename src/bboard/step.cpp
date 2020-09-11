#include <iostream>

#include "bboard.hpp"
#include "step_utility.hpp"

namespace bboard
{

inline void _resetBoardAgentGone(Board* board, const int x, const int y, const int i)
{
    if(board->items[y][x] == Item::AGENT0 + i)
    {
        if(board->HasBomb(x, y))
        {
            board->items[y][x] = Item::BOMB;
        }
        else
        {
            board->items[y][x] = Item::PASSAGE;
        }
    }
}

inline void _setAgent(State* state, const int x, const int y, const int i)
{
    state->items[y][x] = Item::AGENT0 + i;
    state->agents[i].x = x;
    state->agents[i].y = y;
}

void Step(State* state, Move* moves)
{
    // do not execute step on terminal states
    if(state->finished)
        return;

    int aliveAgentsBefore = state->aliveAgents;

    ///////////////////
    //    Flames     //
    ///////////////////
    util::TickFlames(state);

    ///////////////////////
    //  Player Movement  //
    ///////////////////////

    Position oldPos[AGENT_COUNT];
    Position destPos[AGENT_COUNT];
    bool dead[AGENT_COUNT];

    util::FillPositions(state, oldPos);
    util::FillDestPos(state, moves, destPos);
    util::FillAgentDead(state, dead);

    util::FixDestPos<true>(oldPos, destPos, AGENT_COUNT, dead);

    int dependency[AGENT_COUNT] = {-1, -1, -1, -1};
    int roots[AGENT_COUNT] = {-1, -1, -1, -1};

    // the amount of chain roots
    const int rootNumber = util::ResolveDependencies(state, destPos, dependency, roots);

    int rootIdx = 0;
    int i = rootNumber == 0 ? 0 : roots[0]; // no roots -> start from 0

    // ouroboros: every agent wants to move to the current position of a different agent
    // A > B
    // ^   v
    // D < C
    bool ouroboros = rootNumber == 0;

    // iterates 4 times but the index i jumps around the dependencies
    for(int _ = 0; _ < AGENT_COUNT; _++, i = dependency[i])
    {
        if(i == -1)
        {
            rootIdx++;
            i = roots[rootIdx];
        }

        AgentInfo& a = state->agents[i];
        const Move m = moves[i];
        const Position desired = destPos[i];

        if(a.dead || a.ignore)
        {
            continue;
        }
        else if(m == Move::BOMB)
        {
            state->TryPlantBomb<true>(a, i);
            continue;
        }
        else if(m == Move::IDLE || desired == a.GetPos())
        {
            // important: has to be after m == bomb because we won't move when
            // we place a bomb
            continue;
        }

        // the agent wants to move

        if(util::IsOutOfBounds(desired))
        {
            // cannot walk out of bounds
            continue;
        }

        int itemOnDestination = state->items[desired.y][desired.x];

        //if ouroboros, the bomb will be covered by an agent
        if(ouroboros && state->HasBomb(desired.x, desired.y))
        {
            // break ouroboros in case there is a bomb
            itemOnDestination = Item::BOMB;
            // TODO: Do we have to undo the other ouroboros agents?
        }

        //
        // All checks passed - you can try a move now
        //

        // move into flame
        if(IS_FLAME(itemOnDestination))
        {
            state->Kill(i);
            _resetBoardAgentGone(state, a.x, a.y, i);
            continue;
        }

        // collect power-ups (and continue walking)
        if(IS_POWERUP(itemOnDestination))
        {
            util::ConsumePowerup(state->agents[i], itemOnDestination);
            itemOnDestination = Item::PASSAGE;
        }

        // execute move if the destination is free
        // (in the rare case of ouroboros, make the move even
        // if an agent occupies the spot)
        // TODO: ouroboros
        if(itemOnDestination == Item::PASSAGE ||
                (ouroboros && itemOnDestination >= Item::AGENT0))
        {
            // only override the position I came from if it has not been
            // overridden by a different agent that already took this spot
            _resetBoardAgentGone(state, a.x, a.y, i);
            _setAgent(state, desired.x, desired.y, i);
            continue;
        }

        if(itemOnDestination == Item::BOMB)
        {
            // if destination has a bomb & the player has bomb-kick, move the player on it.
            // The idea is to move each player (on the bomb) and afterwards move the bombs.
            // If the bombs can't be moved to their target location, the player that kicked
            // it moves back. Since we have a dependency array we can move back every player
            // that depends on the inital one (and if an agent that moved there this step
            // blocked the bomb we can move him back as well).
            if(state->agents[i].canKick)
            {
                // a player that moves towards a bomb at this(!) point means that
                // there was no DP collision, which means this agent is a root. So we can just
                // override
                // TODO: Remove if?
                _resetBoardAgentGone(state, a.x, a.y, i);
                _setAgent(state, desired.x, desired.y, i);

                // start moving the kicked bomb by setting a velocity
                // the first 5 values of Move and Direction are semantically identical
                Bomb& b = *state->GetBomb(desired.x,  desired.y);
                SetBombDirection(b, Direction(m));
            }
            else
            {
                // allow stepping on bombs because they could move in the next step
                // we'll check bomb movement later and undo this step if necessary
                _resetBoardAgentGone(state, a.x, a.y, i);
                _setAgent(state, desired.x, desired.y, i);
            }
        }
    }

    // Before moving bombs, reset their "moved" flags
    util::ResetBombFlags(state);

    // Fill array of desired positions
    Position bombPositions[state->bombs.count];
    Position bombDestinations[state->bombs.count];
    util::FillBombPositions(state, bombPositions);
    util::FillBombDestPos(state, bombDestinations);

    // TODO: Add again
    // bomb position switching
    // util::FixDestPos<false>(bombPositions, bombDestinations, state->bombs.count);

    // Set bomb directions to idle if they collide with an agent or a static obstacle
    for(int i = 0; i < state->bombs.count; i++)
    {
        Bomb& b = state->bombs[i];
        Position target = bombDestinations[i];

        // check whether the destination is blocked, this means
        // a) The bomb does not move (anymore) and there is an agent on this bomb
        // b) The bomb wanted to move but the destination is blocked (by some agent, powerup or obstacle)
        //    --- note how this excludes bombs because they can still move!
        if(util::BombMovementIsBlocked(state, target))
        {
            // the bomb stops moving
            SetBombDirection(b, Direction::IDLE);
            Position bPos = bombPositions[i];
            bombDestinations[i] = bPos;

            // check whether there is an agent at the origin of this bomb
            int indexAgent = state->GetAgent(bPos.x, bPos.y);
            if(indexAgent > -1
                    // agents which did not move can stay where there are (redundant)
                    // && moves[indexAgent] != Move::IDLE
                    // && moves[indexAgent] != Move::BOMB
                    // if the agents is where he came from he probably got bounced
                    // back to the bomb he was already standing on.
                    && state->agents[indexAgent].GetPos() != oldPos[indexAgent])
            {
                // bounce back the moving agent
                util::AgentBombChainReversion(state, oldPos, moves, bombDestinations, indexAgent);

                // this position is now occupied by the bomb
                if(state->GetAgent(bPos.x, bPos.y) == -1)
                {
                    state->items[bPos.y][bPos.x] = Item::BOMB;
                }
            }
        }
    }

    // Move bombs
    bool bombExplodedDuringMovement = false;
    for(int i = 0; i < state->bombs.count; i++)
    {
        Bomb& b = state->bombs[i];

        if(Move(BMB_DIR(b)) == Move::IDLE)
        {
            if(util::ResolveBombCollision(state, oldPos, moves, bombDestinations, i))
            {
                continue;
            }
        }

        Position pos = bombPositions[i];
        // TODO: We cannot use bombDestinations here because they are not set correctly
        //       anymore at this point. A better solution would be to update the destinations
        //       when the movement of the bomb changes.
        Position target = util::DesiredPosition(b);

        int& oItem = state->items[pos.y][pos.x];
        int& tItem = state->items[target.y][target.x];

        if(util::IsOutOfBounds(target) || IS_STATIC_MOV_BLOCK(tItem))
        {
            // stop moving the bomb
            SetBombDirection(b, Direction::IDLE);
        }
        else if(tItem == Item::FOG)
        {
            // the bomb just disappears when it moves out of range
            if(!state->HasBomb(pos.x, pos.y) && oItem == Item::BOMB)
            {
                oItem = Item::PASSAGE;
            }
            state->bombs.RemoveAt(i);
            i--;
        }
        else
        {
            if(util::ResolveBombCollision(state, oldPos, moves, bombDestinations, i))
            {
                continue;
            }

            // MOVE BOMB
            SetBombPosition(b, target.x, target.y);

            if(!state->HasBomb(pos.x, pos.y) && oItem == Item::BOMB)
            {
                oItem = Item::PASSAGE;
            }

            if(IS_WALKABLE(tItem))
            {
                tItem = Item::BOMB;
            }
            else if(IS_FLAME(tItem))
            {
                // bomb moved into flame -> explode later
                bombExplodedDuringMovement = true;
                SetBombFlag(b, true);
            }
        }
    }

    if(bombExplodedDuringMovement)
    {
        for(int i = 0; i < state->bombs.count; i++)
        {
            if(BMB_FLAG(state->bombs[i]) == int(true))
            {
                state->ExplodeBombAt(i);
                i--;
            }
        }
    }


    ///////////////
    // Explosion //
    ///////////////
    util::TickBombs(state);

    // advance timestep
    state->timeStep++;

    if(aliveAgentsBefore != state->aliveAgents)
    {
        // the number of agents has changed, check if some agent(s) won the game
        util::CheckTerminalState(*state);
    }
}

}
