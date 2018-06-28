#ifndef BBOARD_H_
#define BBOARD_H_

#include <iostream>
#include <string>

namespace bboard
{


const int AGENT_COUNT = 4;
const int BOARD_SIZE = 11;

const int BOMB_LIFETIME = 10;
const int BOMB_DEFAULT_STRENGTH = 1;

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

enum class Direction
{
    IDLE = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT
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
 * @brief Represents any position on a board of a state
 */
struct Position
{
    int x;
    int y;
};

inline bool operator==(const Position& here, const Position& other)
{
    return here.x == other.x && here.y == other.y;
}

inline std::ostream & operator<<(std::ostream & str, const Position& v)
{
    str << "(" << v.x << ", " << v.y << ")";;
    return str;
}

/**
 * @brief The AgentInfo struct holds information ABOUT
 * an agent.
 *
 * - Why not put it in the Agent struct?
 * Because the Agent struct is a virtual base that implements
 * the behaviour of an agent.
 * We might want to hotswap agent behaviours during
 * the game, without having to worry about copying all variables.
 *
 * The act-method is not relevant to the game's mechanics, so
 * it's excluded for now
 *
 * - Why not use an array of vars in the State struct instead?
 * That was the first approach, however fogging the state is a lot
 * easier if all (possibly hidden) data is bundled. Now if someone
 * is out of sight we simply don't expose their AgentInfo to the agent.
 */
struct AgentInfo
{
    int x;
    int y;
    bool dead;

    // power-ups
    bool canKick;
    int bombCount;
    int bombStrength;
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
     * @brief agents Array of all agents and their properties
     */
    AgentInfo agents[AGENT_COUNT];

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
     * @brief canKick Wether or not the agent can kick bombs
     */
    bool canKick[AGENT_COUNT];

    /**
     * @brief bombStrength Holds the bomb strength of all
     * agent's bombs.
     */
    bool bombStrength[AGENT_COUNT];

    /**
     * @brief PutItem Places an item on the board
     */
    void PutItem(int x, int y, Item item)
    {
        board[y][x] = item;
    }

    /**
     * @brief Kill Kills the specified agents
     */
    void Kill(int agentID)
    {
        agents[agentID].dead = true;
    }

    /**
     * Kills all listed agents.
     */
    template<typename... Args>
    void Kill(int agentID, Args... args)
    {
        Kill(agentID);
        Kill(args...);
    }
    /**
     * @brief PutAgents Places agents with given IDs
     * clockwise on the board, starting from top left.
     */
    void PutAgentsInCorners(int a0, int a1, int a2, int a3);

    /**
     * @brief PutAgentsInCorners Places a specified agent
     * on the specified location and updates agent positions
     * @param a0 The agent ID (from 0 to AGENT_COUNT)
     * @param x x-position of the agent.
     * @param y y-position of the agent.
     */
    void PutAgent(int agentID, int x, int y);
};

/**
 * @brief The Agent struct defines a behaviour. For a given
 * state it will return a Move.
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
 * @brief The Bomb struct hold all information about a specific
 * bomb on the board
 */
struct Bomb
{
    Position position;
    Direction velocity = Direction::IDLE; //if the bomb is moving, which way?

    int timeLeft = BOMB_LIFETIME;
    int strength = BOMB_DEFAULT_STRENGTH;
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
