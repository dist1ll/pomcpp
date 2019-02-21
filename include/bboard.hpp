#ifndef BBOARD_H_
#define BBOARD_H_

#include <array>
#include <string>
#include <random>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>

namespace bboard
{

const int MOVE_COUNT  = 4;
const int AGENT_COUNT = 4;
const int BOARD_SIZE  = 11;

static_assert (BOARD_SIZE <= 15, "Board positions must fit into 8-bit");

const int BOMB_LIFETIME = 10;
const int BOMB_DEFAULT_STRENGTH = 1;

const int FLAME_LIFETIME = 4;

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
    PASSAGE    = 0,
    RIGID      = 1,
    WOOD       = 2 << 8,
    BOMB       = 3,
    // optimization I in docs
    FLAMES     = 4 << 16,
    FOG        = 5,
    EXTRABOMB  = 6,
    INCRRANGE  = 7,
    KICK       = 8,
    AGENTDUMMY = 9,
    AGENT0 = (1 << 24),
    AGENT1 = (1 << 24) + 1,
    AGENT2 = (1 << 24) + 2,
    AGENT3 = (1 << 24) + 3
};

inline bool IS_WOOD(int x)
{
    return (((x) >> 8) == 2);
}
inline bool IS_POWERUP(int x)
{
    return ((x) > 5 && (x) < 9);
}
inline bool IS_WALKABLE(int x)
{
    return (IS_POWERUP((x)) || (x) == 0);
}
inline bool IS_FLAME(int x)
{
    return (((x) >> 16) == 4);
}
inline bool IS_AGENT(int x)
{
    return ((x) >= (1 << 24));
}
// bombs can't move through the following `static` objects: walls, boxes and upgrades.
inline bool IS_STATIC_MOV_BLOCK(int x)
{
    return (IS_WOOD((x)) || IS_POWERUP((x)) || ((x) == 1));
}
inline int FLAME_ID(int x)
{
    return (((x) & 0xFFFF) >> 3);
}
inline int FLAME_POWFLAG(int x)
{
    return ((x) & 0b11);
}
inline int WOOD_POWFLAG(int x)
{
    return ((x) & 0b11);
}

/**
 * @brief The FixedQueue struct implements a fixed-size queue,
 * operating on a cicular buffer.
 */
template<typename T, int TSize>
struct FixedQueue
{
    T queue[TSize];
    int index = 0;
    int count = 0;

    int RemainingCapacity()
    {
        return TSize - count;
    }

    /**
     * @brief PopBomb Frees up the position of the elem in the
     * queue to be used by other elems.
     */
    T& PopElem()
    {
        int x = index;
        index = (index + 1) % (TSize);
        count--;
        return queue[x % TSize];
    }

    /**
     * @brief AddElem Adds an element to the queue
     */
    void AddElem(const T& elem)
    {
        NextPos() = elem;
        count++;
    }
    /**
     * @brief RemoveAt Removes an element at a specified index
     * Highly discouraged! Only use if necessary
     */
    void RemoveAt(int removeAt)
    {
        for(int i = removeAt + 1; i < count; i++)
        {
            int translatedIndex = (index + i) % TSize;
            queue[(translatedIndex - 1 + TSize) % TSize] = queue[translatedIndex];
        }
        count--;

    }

    /**
     * @brief PollNext Polls the next free queue spot
     */
    T& NextPos()
    {
        return queue[(index + count) % TSize];
    }

    /**
     * @brief operator [] Circular buffer on all bombs
     * @return The i-th elem if the index is in [0, n]
     * where n = elemcount, sorted w.r.t. their lifetime
     */
    T&       operator[] (const int index);
    const T& operator[] (const int index) const;
};

template<typename T, int TSize>
inline T& FixedQueue<T, TSize>::operator[] (const int offset)
{
    return queue[(index + offset) % TSize];
}
template<typename T, int TSize>
inline const T& FixedQueue<T, TSize>::operator[] (const int offset) const
{
    return queue[(index + offset) % TSize];
}
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
 * the behaviour of an agent. We might want to hotswap agent
 * behaviours during the game, without having to worry about
 * copying all variables.
 *
 * The act-method is not relevant to the game's mechanics, so
 * it's excluded for now
 *
 * - Why not use an array of vars in the State struct instead?
 * That was the first approach, however fogging the state is a
 * lot easier if all (possibly hidden) data is bundled. Now if
 * someone is out of sight we simply don't expose their AgentInfo
 * to the agent.
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

    Position GetPos()
    {
        return {x, y};
    }
};

/**
 * Represents all information about a single
 * bomb on the board.
 *
 * Specification (see docs optimization II)
 *
 *   Bit     Semantics
 * [ 0,  4]  x-Position
 * [ 4,  8]  y-Position
 * [ 8, 12]  ID
 * [12, 16]  Strength
 * [16, 20]  Time
 * [20, 24]  Direction
 */
typedef int Bomb;

// BOMB INFO
// ACCESS ALL PARTS OF THE BOMB INTEGER (EVERYTHING ENCODED
// INTO 4 BIT WIDE FIELDS)
inline int BMB_POS(const Bomb x)
{
    return (((x) & 0xFF));            // [ 0, 8[
}
inline int BMB_POS_X(const Bomb x)
{
    return (((x) & 0xF));             // [ 0, 4[
}
inline int BMB_POS_Y(const Bomb x)
{
    return (((x) & 0xF0) >> 4);       // [ 4, 8[
}
inline int BMB_ID(const Bomb x)
{
    return (((x) & 0xF00) >> 8);      // [ 8,12[
}
inline int BMB_STRENGTH(const Bomb x)
{
    return (((x) & 0xF000) >> 12);    // [12,16[
}
inline int BMB_TIME(const Bomb x)
{
    return (((x) & 0xF0000) >> 16);   // [16,20[
}
inline int BMB_DIR(const Bomb x)
{
    return (((x) & 0xF00000) >> 20);  // [20,24[
}
inline int BMB_MOVED(const Bomb x)
{
    return (((x) & 0xF000000) >> 24); // [24,28[
}

// inverted bit-mask
const int cmask0_4   =  ~0xF;
const int cmask4_8   =  ~0xF0;
const int cmask8_12  =  ~0xF00;
const int cmask12_16 =  ~0xF000;
const int cmask16_20 =  ~0xF0000;
const int cmask20_24 =  ~0xF00000;
const int cmask24_28 =  ~0xF000000;

inline void ReduceBombTimer(Bomb& bomb)
{
    bomb = bomb - (1 << 16);
}
inline void SetBombPosition(Bomb& bomb, int x, int y)
{
    bomb = (bomb & cmask0_4 & cmask4_8) + (x) + (y << 4);
}
inline void SetBombID(Bomb& bomb, int id)
{
    bomb = (bomb & cmask8_12) + (id << 8);
}
inline void SetBombStrength(Bomb& bomb, int strength)
{
    bomb = (bomb & cmask12_16) + (strength << 12);
}
inline void SetBombTime(Bomb& bomb, int time)
{
    bomb = (bomb & cmask16_20) + (time << 16);
}
inline void SetBombDirection(Bomb& bomb, Direction dir)
{
    bomb = (bomb & cmask20_24) + (int(dir) << 20);
}
inline void SetBombMovedFlag(Bomb& bomb, bool moved)
{
    bomb = (bomb & cmask24_28) + (int(moved) << 24);
}

/**
 * @brief The Flame struct holds all information about a specific
 * flame (a Flame represents all fire items generated by a single
 * bomb)
 */
struct Flame
{
    Position position;
    int timeLeft = FLAME_LIFETIME;
    int strength;
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

    /**
     * @brief operator [] This way you can reference a position
     * on the board with a Position (less verbose than board[..][..])
     * @param The position of the board
     * @return The integer reference at the correct board position
     */
    int& operator[] (const Position& pos);

    int board[BOARD_SIZE][BOARD_SIZE];

    int timeStep = 0;
    int aliveAgents = AGENT_COUNT;

    /**
     * @brief agents Array of all agents and their properties
     */
    AgentInfo agents[AGENT_COUNT];

    /**
     * @brief bombQueue Holds all bombs on this board
     */
    FixedQueue<Bomb, MAX_BOMBS> bombs;

    /**
     * @brief flames Holds all flames on this board
     */
    FixedQueue<Flame, MAX_BOMBS> flames;

    /**
     * @brief PlantBomb Plants a bomb at the given position.
     * Does not add a bomb to the queue if the agent maxed out.
     * @param id Agent that plants the bomb
     * @param x X position of the bomb
     * @param y Y position of the bomb
     * @param setItem Should the bomb item be set on that position
     */
    void PlantBomb(int x, int y, int id, bool setItem =  false);

    /**
     * @brief PlantBombModifiedLife Same as PlantBomb with the option
     * to set the lifetime of the bomb.
     */
    void PlantBombModifiedLife(int x, int y, int id, int lifeTime = BOMB_LIFETIME, bool setItem = false);

    /**
     * @brief ExplodeTopBomb Explodes the bomb at the top of the
     * queue and subsequently spawns flames. Handles "dead" bombs,
     * edge conditions etc.
     */
    void ExplodeTopBomb();

    /**
     * @brief ExplodeTopBomb Explodes the bomb at the the specified index
     * of the queue and spawns flames. This method is less performant
     * than ExplodeTopBomb, so try to use only when necessary
     */
    void ExplodeBombAt(int index);

    /**
     * @brief hasBomb Returns true if a bomb is at the specified
     * position
     */
    bool HasBomb(int x, int y);

    /**
     * @brief GetBomb Returns a bomb at the specified location or 0 if
     * no bomb has that position
     */
    Bomb* GetBomb(int x, int y);

    /**
     * @brief HasAgent Returns the index of the agent that occupies
     * the given position. -1 if no agent is there
     */
    int GetAgent(int x, int y);

    /**
     * @brief GetBombIndex If a bomb is at position (x,y), then
     * returns the index of the bomb in the bomb queue. -1 otherwise
     */
    int GetBombIndex(int x, int y);

    /**
     * @brief SpawnFlames Spawns rays of flames at the
     * specified location.
     * @param x The x position of the origin of flames
     * @param y The y position of the origin of flames
     * @param strength The farthest reachable distance
     * from the origin
     */
    void SpawnFlame(int x, int y, int strength);

    /**
     * @brief PopFlame extinguishes the top flame
     * of the flame queue.
     */
    void PopFlame();

    /**
     * @brief PutItem Places an item on the board
     */
    inline void PutItem(int x, int y, Item item)
    {
        board[y][x] = item;
    }

    /**
     * @brief BreakWood Returns the correct powerup
     * for the given pow-flag
     */
    Item FlagItem(int powFlag);

    /**
     * @brief Kill Kills the specified agents
     */
    void Kill(int agentID)
    {
        if(!agents[agentID].dead)
        {
            agents[agentID].dead = true;
            aliveAgents--;
        }
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
    void PutAgent(int x, int y, int agentID);
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

    int id = -1;

    /**
     * This method defines the behaviour of the Agent.
     * Classes that implement Agent can be used to participate
     * in a game and run.
     *
     * @brief For a given state, return a Move
     * @param state The (potentially fogged) board state
     * @return A Move (integer, 0-..)
     */
    virtual Move act(const State* state) = 0;
};


/**
 * @brief The Environment struct holds all information about a
 * Game (current state, participating agents) and takes care of
 * distributing observations to the correct agents.
 */
class Environment
{

private:

    std::unique_ptr<State> state;
    std::array<Agent*, AGENT_COUNT> agents;
    std::function<void(const Environment&)> listener;

    // Current State
    bool finished = false;
    bool hasStarted = false;
    bool isDraw = false;

    int agentWon = -1; // FFA
    int teamWon = -1; // Team

    bool threading = false;
    int threadCount = 1;

    Move lastMoves[AGENT_COUNT];

public:

    Environment();
    /**
     * @brief MakeGame Initializes the state
     */
    void MakeGame(std::array<Agent*, AGENT_COUNT> a);

    /**
     * @brief StartGame starts a game and prints in the terminal output
     * (blocking)
     * @param timeSteps maximum of time steps after which the game ends
     * @param render True if the game should be rendered (and played out with
     * delay)
     * @param stepByStep For debugging purposes.If true, pauses execution
     * after each step. (Press enter to continue)
     */
    void StartGame(int timeSteps, bool render=true, bool stepByStep = false);

    /**
     * @brief Step Executes a step, given by the params
     * @param competitiveTimeLimit Set to true if the agents
     * need to produce a response in less than 100ms (competition
     * rule). Timed out agents will have the IDLE move.
     */
    void Step(bool competitiveTimeLimit = false);

    /**
     * @brief Print Pretty-prints the Environment
     * @param clear Should the console be cleared first?
     */
    void Print(bool clear = true);

    /**
     * @brief GetState Returns a reference to the current state of
     * the environment
     */
    State& GetState() const;

    /**
     * @brief SetAgents Registers all agents that will participate
     * in this game
     * @param a An array of agent pointers (with correct length)
     */
    void SetAgents(std::array<Agent*, AGENT_COUNT> agents);

    /**
     * @brief GetAgent
     * @param agentID
     * @return
     */
    Agent* GetAgent(uint agentID) const;

    /**
     * @brief SetStepListener Sets the step listener. Step listener
     * gets invoked every time after the step function was called.
     */
    void SetStepListener(const std::function<void(const Environment&)>& f);

    /**
     * @return True if the last step ended the current game
     */
    bool IsDone();

    /**
     * @brief IsDraw Did the game end in a draw?
     */
    bool IsDraw();

    /**
     * @brief GetWinner If the game was won by someone, return
     * the agent's ID that won
     */
    int GetWinner();

    /**
     * @brief GetLastMove Returns the last move made by the given agent.
     * (Disregard if agent is dead)
     */
    Move GetLastMove(int agentID);

};

/**
 * @brief InitBoardItems Puts boxes, rigid objects and powerups on
 * the field without adding/creating agents
 * @param seed The random seed for the item generator
 */
void InitBoardItems(State& state, int seed = 0x1337);

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
void PrintState(State* state, bool clearConsole = false);

/**
 * @brief Returns a string, corresponding to the given item
 * @return A 3-character long string
 */
std::string PrintItem(int item);

}

namespace std
{
template<>
struct hash<bboard::Position>
{
    size_t
    operator()(const bboard::Position & obj) const
    {
        return hash<int>()(obj.x + obj.y * bboard::BOARD_SIZE);
    }
};
}

#endif
