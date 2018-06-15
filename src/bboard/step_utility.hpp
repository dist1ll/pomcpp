#ifndef STEP_UTILITY_H
#define STEP_UTILITY_H

#include "bboard.hpp"

namespace bboard
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
 * Consider an 2d array (referred to as dependencies) d[AGENTS][2],
 * where d[i] = j <==> agent i want's agent j's spot (and j wants to
 * go somewhere else)
 *
 * If d[i] = j, we can say agent i depends on where agent j goes
 * and write i --> j. This is because i will only be able to go
 * to j's spot once it's confirmed that j can move to his destination.
 *
 * What if j depends on a different agent k? We can list the
 * dependcy as a chain:
 *
 * i --> j --> k --> l
 *
 * If we resolve l, we can determine if k can move. That means resolving
 * backwards we can consecutively decide the movement validity in isolation.
 *
 * If we detect a circular dependency, the array can stay the same
 *
 * @brief ResolveDependencies corrects the order of the collision
 * dependency array
 * @param array The dependency array of all agents
 */
void ResolveDependencies(int array[][BOARD_SIZE]);

}

#endif // STEP_UTILITY_H
