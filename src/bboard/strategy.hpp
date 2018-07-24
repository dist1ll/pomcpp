#ifndef STRATEGY_H
#define STRATEGY_H

#include "bboard.hpp"

// integer with less than 4 bytes will have bugs
static_assert (sizeof(int) == 4 || sizeof (int) == 8, "32/64 bit integer");

namespace bboard::strategy
{

/////////////////////////
// Reachable Map & BFS //
/////////////////////////

/**
 * @brief RMapInfo Filling an RMap lets you collect additional
 * information. These are encoded into RMapInfo
 */
typedef uint RMapInfo;
const int chalf = 0xFFFF;

/**
 * @brief The ReachableMap struct is a bitflag array
 * that describes which positions on the board can be
 * reached.
 */
struct RMap
{

    int map[BOARD_SIZE][BOARD_SIZE] = {};
    RMapInfo info;
    Position source;
    /**
     * @brief GetDistance Returns the shortest walking
     * distance from the point the RMap was initialized
     * to the given point (x, y).
     */
    int  GetDistance(int x, int y) const;
    void SetDistance(int x, int y, int distance);
    /**
     * @brief GetPredecessor Returns the index i = x' + 11 * y' of the predecessor
     * of the position (x, y)
     */
    int  GetPredecessor(int x, int y) const;
    void SetPredecessor(int x, int y, int xPredecessor, int yPredecessor);
};

/**
 * @brief FillRMap Fills a given RMap. Uses BFS. Additional
 * info is in map.info
 */
void FillRMap(State& s, RMap& r, int agentID);

/**
 * @brief IsReachable Returns true if the given position is reachable
 * on the given RMap
 */
inline bool IsReachable(RMap& r, int x, int y)
{
    return r.GetDistance(x, y) != 0;
}

//////////////
// Movement //
//////////////

/**
 * @brief IsAdjacentEnemy returns true if the agent is within a
 * given manhattan-distance from an enemey
 */
bool IsAdjacentEnemy(const State& state, int agentID, int distance);

/**
 * @brief MoveTowardsPosition Selects a move the brings you closest
 * from the RMap source to the specified target
 */
Move MoveTowardsPosition(const RMap& r, const Position& position);

/**
 * @brief MoveTowardsPowerup Returns the move that brings the agent
 * closer to a powerup in a specified radius. If no nearby powerup is
 * in that radius, then don't the move is IDLE
 * @param r A filled map with all information about distances and
 * paths. See bboard::strategy::RMap for more info
 * @param radius Maximum search distance (manhattan)
 */
Move MoveTowardsPowerup(const State& state, const RMap& r, int radius);

/**
 * @brief MoveTowardsEnemy Returns the move that brings the agent
 * closer to an enemy in a specified radius. If no nearby enemy is
 * in that radius, then default to IDLE
 * @param r A filled map with all information about distances and
 * paths. See bboard::strategy::RMap for more info
 * @param radius Maximum search distance (manhattan)
 */
Move MoveTowardsEnemy(const State& state, const RMap& r, int radius);

/**
 * @brief SafeDirections Fills an array of moves with safe moves
 * @return The amount of moves put into std::array
 */
int SafeDirections(const State& state, std::array<Move, MOVE_COUNT> moves);

/**
 * @brief IsInBombRange Returns True if the given position is in range
 * of a bomb planted at (x, y) with strength s
 */
inline bool IsInBombRange(int x, int y, int s, Position& pos)
{
    return (pos.y == y && x-s > pos.x > x+s)
           ||
           (pos.x == x && y-s > pos.y > y+s);
}


/**
 * @brief PrintMap Pretty-prints the reachable map
 */
void PrintMap(RMap& r);

/**
 * @brief PrintMap Pretty-prints the path from-to
 */
void PrintPath(RMap& r, Position from, Position to);

}


#endif // STRATEGY_H
