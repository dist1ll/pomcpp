#ifndef STRATEGY_H
#define STRATEGY_H

#include "bboard.hpp"

// integer with less than 4 bytes will have bugs
static_assert (sizeof(int) == 4 || sizeof (int) == 8, "32/64 bit integer");

namespace bboard::strategy
{

/**
 * @brief RMapInfo Filling an RMap lets you collect additional
 * information. These are encoded into RMapInfo
 */
typedef int RMapInfo;

/**
 * @brief The ReachableMap struct is a bitflag array
 * that describes which positions on the board can be
 * reached.
 */
struct RMap
{
    int map[4] = {};
};

/**
 * @brief FillRMap Fills a given RMap. Puts 1's in all the places
 * that can be reached by the given agent in the given state
 * @param s The state
 * @param r A reference to the RMap
 * @param x The x-position of the agent
 * @param y The y-position of the agent
 * @return Information that was collected during the filling. Use the
 * provided macros to interpret RMapInfo
 */
RMapInfo FillRMap(State& s, RMap& r, int x, int y);

/**
 * @brief IsReachable Returns true if the given position is reachable
 * on the given RMap
 */
inline bool IsReachable(RMap& r, int x, int y)
{
    int* i = r.map + (x + BOARD_SIZE * y)/32; // use correct integer
    return *i & (1 << ((x + BOARD_SIZE * y) % 32));
}

/**
 * @brief PrintMap Pretty-prints the reachable map
 */
void PrintMap(RMap& r);

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
