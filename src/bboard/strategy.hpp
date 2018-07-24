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

const int chalf = 0xFFFF;

/**
 * @brief The ReachableMap struct is a bitflag array
 * that describes which positions on the board can be
 * reached.
 */
struct RMap
{
    int map[BOARD_SIZE][BOARD_SIZE] = {};

    /**
     * @brief GetDistance Returns the shortest walking
     * distance from the point the RMap was initialized
     * to the given point (x, y).
     */
    int  GetDistance(int x, int y);
    void SetDistance(int x, int y, int distance);

    /**
     * @brief GetPredecessor Returns the index i = x' + 11 * y' of the predecessor
     * of the position (x, y)
     */
    int  GetPredecessor(int x, int y);
    void SetPredecessor(int x, int y, int xPredecessor, int yPredecessor);
};

/**
 * @brief FillRMap Fills a given RMap. Optimized Dijkstra.
 * @param s The state
 * @param r A reference to the RMap
 * @param x The x-position of the agent
 * @param y The y-position of the agent
 * @param _distance ignore.
 * @return Information that was collected during the filling. Use the
 * provided macros to interpret RMapInfo
 */
RMapInfo FillRMap(State& s, RMap& r, int x, int y, int _distance = 0);

/**
 * @brief IsReachable Returns true if the given position is reachable
 * on the given RMap
 */
inline bool IsReachable(RMap& r, int x, int y)
{
    return r.GetDistance(x, y) != 0;
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

}


#endif // STRATEGY_H
