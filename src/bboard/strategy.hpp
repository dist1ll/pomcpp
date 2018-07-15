#ifndef STRATEGY_H
#define STRATEGY_H

#include "bboard.hpp"

namespace bboard::strategy
{

/**
 * @brief IsAdjacentEnemy returns true if the agent is within a
 * given manhattan-distance from an enemey
 */
bool IsAdjacentEnemy(const State& state, int agentID, int distance);

}


#endif // STRATEGY_H
