#ifndef STEP_UTILITY_H
#define STEP_UTILITY_H

#include "bboard.hpp"

namespace bboard::util
{

/**
 * @brief DesiredPosition returns the x and y values of the agents
 * destination
 * @param x Current agents x position
 * @param y Current agents y position
 * @param m The desired move
 */
Position DesiredPosition(int x, int y, Move m);

/**
 * @brief DesiredPosition moves in the opposite direction of the given Move
 * Which position was the agent in if he made the Move m and landed in (x,y)?
 */
Position OriginPosition(int x, int y, Move m);

/**
 * @brief DesiredPosition Returns the target position of the bomb. If it's
 * idle, the desired position will be its current position
 */
Position DesiredPosition(const Bomb b);

/**
 * @brief RevertAgentMove Moves back a specified agent and bounces back every
 * agent or bomb that stands in its way recursively.
 * The bomb dest is an array of desired destination positions of bombs. That way
 * we don't need to read the direction and find out if they've been alraedy moved
 * @return The position of the last agent/bomb that was bounced back in the chain.
 */
Position AgentBombChainReversion(State* state, Position oldAgentPos[AGENT_COUNT], Move moves[AGENT_COUNT],
                                 Position bombDest[MAX_BOMBS], int agentID);

/**
 * @brief FillPositions Fills an array of Positions with positions of
 * all agents of the given state.
 */
void FillPositions(const State* state, Position p[AGENT_COUNT]);

/**
 * @brief FillDestPos Fills an array of destination positions.
 * @param state The state
 * @param m An array of all agent moves
 * @param p The array to be filled wih dest positions
 */
void FillDestPos(const State* state, Move m[AGENT_COUNT], Position p[AGENT_COUNT]);

/**
 * @brief FillBombDestPos Fills the given array p with all desired bomb
 * positions that moving bombs are anticipating
 */
void FillBombDestPos(const Board* board, Position p[MAX_BOMBS]);

void FillAgentDead(const State* state, bool dead[AGENT_COUNT]);

template <int numElem, int sizePos>
void FixDestPos(bool skip[numElem], Position o[sizePos], Position d[sizePos])
{
    bool foundCollision;
    bool fixDest[numElem];
    std::fill_n(fixDest, numElem, false);

    // TODO: Maybe there is a better way than looping
    do
    {
        foundCollision = false;
        for(int i = 0; i < numElem; i++)
        {
            // skip
            if (skip[i])
                continue;

            for(int j = i + 1; j < numElem; j++)
            {
                // skip
                if (skip[j])
                    continue;

                // forbid moving to the same position and switching positions
                if(d[i] == d[j] || (d[i].x == o[j].x && d[i].y == o[j].y &&
                        d[j].x == o[i].x && d[j].y == o[i].y))
                {
                    foundCollision = true;
                    fixDest[i] = true;
                    fixDest[j] = true;
                }
            }
        }

        for (int i = 0; i < numElem; i++) {
            if(fixDest[i]) {
                d[i] = o[i];
            }
        }
    } while(foundCollision);
}

/**
 * TODO: Fill doc for dependency resolving
 *
 */
int ResolveDependencies(const State* state, Position des[AGENT_COUNT],
                        int dependency[AGENT_COUNT], int chain[AGENT_COUNT]);

/**
 * @brief TickFlames Counts down all flames in the flame queue
 * (and possible extinguishes the flame)
 */
void TickFlames(Board* board);

/**
 * @brief TickBombs Counts down all bomb timers and explodes them
 * if they arrive at 10
 */
void TickBombs(Board* board);

/**
 * @brief MoveBombsForward moves all bombs forward that have been
 * kicked before by 1 position.
 */
void MoveBombsForward(Board* board);

/**
 * @brief ConsumePowerup Lets an agent consume a powerup
 * @param info The agentInfo of the agent which consumes the item.
 * @param powerUp A powerup item. If it's something else,
 * this function will do nothing.
 */
void ConsumePowerup(AgentInfo& info, int powerUp);

/**
 * @brief PrintDependency Prints a dependency array in a nice
 * format (stdout)
 * @param dependency Integer array that contains the dependencies
 */
void PrintDependency(int dependency[AGENT_COUNT]);

/**
 * @brief PrintDependencyChain Prints a dependency chain in a nice
 * format (stdout)
 * @param dependency Integer array that contains the dependencies
 * @param chain Integer array that contains all chain roots.
 * (-1 is a skip)
 */
void PrintDependencyChain(int dependency[AGENT_COUNT], int chain[AGENT_COUNT]);

/**
 * @brief HasDPCollision Checks if the given agent has a destination
 * position collision with another agent
 * @param The agent that's checked for collisions
 * @return True if there is at least one collision
 */
bool HasDPCollision(const State* state, Position dp[AGENT_COUNT], int agentID);

/**
 * @brief HasBombCollision Checks whether a bomb collides with another bomb
 * on the board
 * @param index Only bombs with a queue index larger or equal to `index` will be
 * considered
 * @return True if the given bomb collides with another bomb
 */
bool HasBombCollision(const Board* board, const Bomb& b, int index = 0);

/**
 * @brief ResolveBombCollision Checks if a specified bomb collides
 * with another bomb(s). If that's the case, any bombs participating in the collision
 * will keep their position and if the bomb was kicked in this round the agents
 * will be bounced back to their old position (alongside any agents that moved
 * to that agents spot in the meantime)
 * @param index Only bombs with a queue index larger or equal to `index` will be
 * considered
 */
void ResolveBombCollision(State* state, Position oldAgentPos[AGENT_COUNT], Move moves[AGENT_COUNT],
                          Position bombDest[MAX_BOMBS], int index = 0);

/**
 * @brief ResetBombFlags Resets the "moved" flag of each bomb on the board
 * back to false.
 */
void ResetBombFlags(Board* board);

/**
 * @brief IsOutOfBounds Checks wether a given position is out of bounds
 */
inline bool IsOutOfBounds(const Position& pos)
{
    return pos.x < 0 || pos.y < 0 || pos.x >= BOARD_SIZE || pos.y >= BOARD_SIZE;
}

/**
 * @brief IsOutOfBounds Checks wether a given position is out of bounds
 */
inline bool IsOutOfBounds(const int& x, const int& y)
{
    return x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE;
}

/**
 * @brief BombMovementIsBlocked Checks whether the bomb movement to the specified
 * target is blocked (not possible)
 */
inline bool BombMovementIsBlocked(const Board* board, Position target)
{
    return IsOutOfBounds(target)
            || IS_STATIC_MOV_BLOCK(board->items[target.y][target.x])
            || IS_AGENT(board->items[target.y][target.x]);
}

/**
 * @brief GetWinningTeam Get the winning team of the provided state.
 * @param state The state
 * @return 0 when there is no winner or the winner is in no team, team
 * id of the winning team otherwise
 */
int GetWinningTeam(const State& state);

/**
 * @brief CheckTerminalState Checks whether the state is a terminal state
 * (some agent/team won) and updates the state attributes accordingly.
 * @param state The state
 */
void CheckTerminalState(State& state);

/**
 * @brief CompareTimeLeft Checks whether timeLeft of lhs is smaller than
 * timeLeft of rhs.
 * @param lhs A flame object
 * @param rhs A flame object
 * @return lhs.timeLeft < rhs.timeLeft
 */
bool CompareTimeLeft(const Flame& lhs, const Flame& rhs);

/**
 * @brief OptimizeFlameQueue Optimizes (ordering + board lookup ids) an
 * potentionally unordered flame queue for faster step processing.
 * @param board The board used to save flame ids
 * @param flames The flame queue which should be optimized
 * @return the remaining timeLeft
 */
int OptimizeFlameQueue(Board& board);

}

#endif // STEP_UTILITY_H
