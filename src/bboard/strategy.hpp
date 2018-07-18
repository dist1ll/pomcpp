#ifndef STRATEGY_H
#define STRATEGY_H

#include "bboard.hpp"

// integer with less than 4 bytes will have bugs
static_assert (sizeof(int) == 4 || sizeof (int) == 8, "32/64 bit integer");

namespace bboard::strategy
{

struct DijkstraMap
{

};

/**
 * @brief IsAdjacentEnemy returns true if the agent is within a
 * given manhattan-distance from an enemey
 */
bool IsAdjacentEnemy(const State& state, int agentID, int distance);

/**
 * @brief CanSafelyBomb returns true if the agent can lay a bomb
 * (enough ammo) and wouldn't get stuck by that bomb.
 * @return
 */
bool CanSafelyBomb(const State& state, int agentID);

}


#endif // STRATEGY_H
