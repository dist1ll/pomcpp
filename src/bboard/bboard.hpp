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

const int MAX_BOMBS_PER_AGENT = 5;
const int MAX_BOMBS = AGENT_COUNT * MAX_BOMBS_PER_AGENT;

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

    // power-ups
    int bombCount = 0;
    int maxBombCount = 1;
    int bombStrength = BOMB_DEFAULT_STRENGTH;

    bool canKick = false;
    bool dead = false;
};

/**
 * @brief The Bomb struct hold all information about a specific
 * bomb on the board
 */
struct Bomb
{
    Position position;
    Direction velocity = Direction::IDLE; //if the bomb is moving, which way?
    int id; // debug purposes
    int timeLeft = BOMB_LIFETIME;
    int strength = BOMB_DEFAULT_STRENGTH;
};

/**
 * @brief The BombQueue struct holds bombs in a fixed-size queue.
 * You can insert bombs and remove the front-most bomb.
 *
 * You can also iterate over all bombs with the subscript operator
 */
struct BombQueue
{
    Bomb _allbmbs[MAX_BOMBS];

    int bombsOnBoard = 0;
    int startingIndex = 0;

    /**
     * @brief PopBomb Frees up the position of the bomb in the
     * queue to be used by other bombs.
     */
    inline void PopBomb()
    {
        startingIndex = (startingIndex + 1) % (MAX_BOMBS);
        bombsOnBoard--;
    }
    /**
     * @brief PollNext Polls the next free queue spot
     */
    inline Bomb& NextPos()
    {
        return _allbmbs[(startingIndex + bombsOnBoard) % MAX_BOMBS];
    }

    /**
     * @brief operator [] Circular buffer on all bombs
     * @return The i-th bomb if the index is in [0, n]
     * where n = bombsOnBoard, sorted w.r.t. their lifetime
     */
    Bomb& operator[] (const int index);
};

inline Bomb& BombQueue::operator[] (const int index)
{
    return _allbmbs[(startingIndex + index) % MAX_BOMBS];
}

/**
 * Represents all information associated with the game board.
 * Includes (in)destructible obstacles, bombs, player positions,
 * etc (as defined by the Pommerman source)
 *
 * @brief Holds all information about the board
 */
struct State
{

    /**
     * @brief operator [] This way you can reference a position
     * on the board with a Position (less verbose than board[..][..])
     * @param The position of the board
     * @return The integer reference at the correct board position
     */
    int& operator[] (const Position& pos);

    int board[11][11];

    /**
     * @brief agents Array of all agents and their properties
     */
    AgentInfo agents[AGENT_COUNT];

    /**
     * @brief bombs Holds all bombs on this board
     */
    BombQueue bombQueue;

    /**
     * @brief PlantBomb Plants a bomb at the given position.
     * Does not add a bomb to the queue if the agent maxed out.
     * @param id Agent that plants the bomb
     * @param x X position of the bomb
     * @param y Y position of the bomb
     */
    void PlantBomb(int id, int x, int y);

    /**
     * @brief hasBomb Returns true if a bomb is at the specified
     * position
     */
    bool HasBomb(int x, int y);

    /**
     * @brief Proxy for BombQueue::PopBomb()
     */
    void PopBomb();

    /**
     * @brief SpawnFlames Spawns rays of flames at the
     * specified location.
     * @param x The x position of the origin of flames
     * @param y The y position of the origin of flames
     * @param strength The farthest reachable distance
     * from the origin
     */
    void SpawnFlames(int x, int y, int strength);

    /**
     * @brief PutItem Places an item on the board
     */
    inline void PutItem(int x, int y, Item item)
    {
        board[y][x] = item;
    }

    /**
     * @brief Kill Kills the specified agents
     */
    inline void Kill(int agentID)
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

inline int& State::operator[] (const Position& pos)
{
    return board[pos.y][pos.x];
}

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
 * @brief InitState Returns an meaningfully initialized state
 * @param state State
 * @param a0 Agent no. that should be at top left
 * @param a1 Agent no. that should be top right
 * @param a2 Agent no. that should be bottom right
 * @param a3 Agent no. that should be bottom left
 */
void InitState(State* state, int a0, int a1, int a2, int a3);

/**
 * @brief Applies given moves to the given board state.
 * @param state The state of the board
 * @param moves Array of 4 moves
 */
void Step(State* state, Move* moves);

/**
 * @brief StartGame starts a game and prints in the terminal output
 * (blocking)
 * @param state The initial state of the game
 * @param agents Array of agents that participate in this game
 * @param timeSteps maximum of time steps after which the game ends
 */
void StartGame(State* state, Agent* agents[AGENT_COUNT], int timeSteps);

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
