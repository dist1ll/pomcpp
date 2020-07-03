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
Position AgentBombChainReversion(State& state, Move moves[AGENT_COUNT],
                                 Position bombDest[MAX_BOMBS], int agentID);

/**
 * @brief FillPositions Fills an array of Positions with positions of
 * all agents of the given state.
 */
void FillPositions(State* s, Position p[AGENT_COUNT]);

/**
 * @brief FillDestPos Fills an array of destination positions.
 * @param s The State
 * @param m An array of all agent moves
 * @param p The array to be filled wih dest positions
 */
void FillDestPos(State* s, Move m[AGENT_COUNT], Position p[AGENT_COUNT]);

/**
 * @brief FillBombDestPos Fills the given array p with all desired bomb
 * positions that moving bombs are anticipating
 */
void FillBombDestPos(State* s, Position p[MAX_BOMBS]);

/**
 * @brief FixDestPos Reverts the desired positions if the agents want
 * to switch places or move to the same position.
 * @param s The state
 * @param desiredPositions an array of desired positions
 */
void FixDestPos(State* s, Position desiredPositions[AGENT_COUNT]);

/**
 * TODO: Fill doc for dependency resolving
 *
 */
int ResolveDependencies(State* s, Position des[AGENT_COUNT],
                        int dependency[AGENT_COUNT], int chain[AGENT_COUNT]);

/**
 * @brief TickFlames Counts down all flames in the flame queue
 * (and possible extinguishes the flame)
 */
void TickFlames(State& state);

/**
 * @brief TickBombs Counts down all bomb timers and explodes them
 * if they arrive at 10
 */
void TickBombs(State& state);

/**
 * @brief MoveBombsForward moves all bombs forward that have been
 * kicked before by 1 position.
 */
void MoveBombsForward(State& state);

/**
 * @brief ConsumePowerup Lets an agent consume a powerup
 * @param agentID The agent's ID that consumes the item
 * @param powerUp A powerup item. If it's something else,
 * this function will do nothing.
 */
void ConsumePowerup(State& state, int agentID, int powerUp);

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
bool HasDPCollision(const State& state, Position dp[AGENT_COUNT], int agentID);

/**
 * @brief HasBombCollision Checks wether a bomb collides with another bomb
 * on the board
 * @param index Only bombs with a queue index larger or equal to `index` will be
 * considered
 * @return True if the given bomb collides with another bomb
 */
bool HasBombCollision(const State& state, const Bomb& b, int index = 0);

/**
 * @brief ResolveBombMovementollision Checks if a specified bomb collides
 * with another bomb(s). If that's the case, any bombs participating in the collision
 * will keep their position and if the bomb was kicked in this round the agents
 * will be bounced back to their old position (alongside any agents that moved
 * to that agents spot in the meantime)
 * @param index Only bombs with a queue index larger or equal to `index` will be
 * considered
 */
void ResolveBombCollision(State& state, Move moves[AGENT_COUNT],
                          Position bombDest[MAX_BOMBS], int index = 0);

/**
 * @brief ResetBombFlags Resets the "moved" flag of each bomb in the state
 * back to false.
 */
void ResetBombFlags(State& state);

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

}

#endif // STEP_UTILITY_H
