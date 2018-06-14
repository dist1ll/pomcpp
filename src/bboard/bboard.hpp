#ifndef BBOARD_H_
#define BBOARD_H_

#include <string>

namespace bboard
{


const int AGENT_COUNT = 4;
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

enum Item
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

    /**
     * @brief agentX The x-positions of all 4 agents
     */
    int agentX[AGENT_COUNT];

    /**
     * @brief agentY The y-positions of all 4 agents
     */
    int agentY[AGENT_COUNT];

    /**
     * @brief dead dead[i] = Agent i is dead
     */
    bool dead[AGENT_COUNT];

    /**
     * @brief PutItem Places an item on the board
     */
    inline void PutItem(int x, int y, Item item)
    {
        board[x][y] = item;
    }

    /**
     * @brief PutAgents Places agents with given IDs
     * clockwise on the board, starting from top left.
     */
    void PutAgents(int a0, int a1, int a2, int a3);
};

/**
 * @brief The Agent struct defines a behaviour
 */
struct Agent
{
    virtual ~Agent() {}

    /**
     * This method defines the behaviour of the Agent.
     * Classes that implement Agent can be used to participate
     * in a game and run.
     *
     * @brief For a given state, return a Move
     * @param state The (potentially fogged) board state
     * @return A Move (integer, 0-..)
     */
    virtual Move act(State* state) = 0;
};

/**
 * @brief Represents any position on a board of a state
 */
struct Position
{
    int x;
    int y;

    bool operator==(const Position& other)
    {
        return x == other.x && y == other.y;
    }
};


/**
 * @brief Same as init state but without obstacles
 * @see bboard::InitState
 */
State* InitEmpty(int a0, int a1, int a2, int a3);

/**
 * @brief InitState Returns an meaningfully initialized state
 * @param a0 Agent no. that should be at top left
 * @param a1 Agent no. that should be top right
 * @param a2 Agent no. that should be bottom right
 * @param a3 Agent no. that should be bottom left
 */
State* InitState(int a0, int a1, int a2, int a3);

/**
 * @brief Applies given moves to the given board state.
 * @param state The state of the board
 * @param moves Array of 4 moves
 */
void Step(State* state, Move* moves);


/**
 * @brief DesiredPosition returns the x and y values of the agents
 * destination
 * @param x Current agents x position
 * @param y Current agents y position
 * @param m The desired move
 */
Position DesiredPosition(int x, int y, Move m);

/**
 * @brief Prints the state into the standard output stream.
 * @param state The state to print
 */
void PrintState(State* state);

/**
 * @brief Returns a string, corresponding to the given item
 * @return A 3-character long string
 */
std::string PrintItem(int item);

}

#endif
