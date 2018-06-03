#ifndef BBOARD_H_
#define BBOARD_H_

namespace bboard
{


const int BOARD_SIZE = 11;

/**
 * Holds all moves an agent can make on a board. An array
 * of 4 moves are necessary to correctly calculate a full
 * simulation step of the board
 * @brief Represents an atomic move on a board
 */
enum class Move
{
    IDLE = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    BOMB
};

enum class Item
{
    PASSAGE = 0,
    RIGID,
    WOOD,
    BOMB,
    FLAMES,
    FOG,
    EXTRABOMB,
    INCRRANGE,
    KICK,
    AGENTDUMMY,
    AGENT0,
    AGENT1,
    AGENT2,
    AGENT3
};

/**
 * Represents all information associated with the game board.
 * Includes (in)destructible obstacles, bombs, player positions,
 * etc (as defined by the Pommerman source)
 *
 * @brief Holds all information about the board
 */
struct State
{
    int board[11][11];
};

/**
 * @brief Represents any position on a board of a state
 */
struct Position
{
    int x;
    int y;
};


/**
 * @brief InitState
 * @return Returns an instance of State
 */
State* InitState(Position* positions);

/**
 * @brief Applies given moves to the given board state.
 * @param s The state of the board
 * @param m Array of 4 moves
 */
void Step(State* s, Move* m);

/**
 * @brief Prints the state into the standard output stream.
 * @param s The state to print
 */
void PrintState(State* s);

}

#endif
