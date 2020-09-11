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

    // resolve conflicting bomb destinations (and reset affected agents)
    util::ResolveBombMovement(state, oldPos);

    // move the bombs (bombs can also explode if they move into flames)
    util::MoveBombs(state);

    // let bombs explode due to timeouts
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
