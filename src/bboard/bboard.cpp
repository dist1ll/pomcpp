#include <random>
#include <chrono>
#include <thread>
#include <iostream>

#include "bboard.hpp"
#include "colors.hpp"

namespace bboard
{

/////////////////////////
// Auxiliary Functions //
/////////////////////////

/**
 * @brief _cleanFlameSpawnPosition Checks whether a flame can be spawned at the specified position (x, y).
 * If there exists a flame object at (x, y) with a different timeLeft, the existing flame is removed.
 *
 * @param flames The flames of the state
 * @param boardItem The board item at (x, y)
 * @param x The x coordinate
 * @param y The y coordinate
 * @param timeStep The current timestep
 * @param outSpawnFlame Returns whether we can spawn a new flame at (x, y)
 * @param outContinueFlameSpawn Returns whether the flame spawning can be continued (abort when encountering a destroyed wood block)
 */
template <int count>
void _cleanFlameSpawnPosition(FixedQueue<Flame, count>& flames, const int boardItem, const int x, const int y, const int timeStep, bool& outSpawnFlame, bool& outContinueFlameSpawn)
{
    if(IS_FLAME(boardItem))
    {
        // find the old flame object for this position
        int flameId = bboard::FLAME_ID(boardItem);
        // start from the back to save time finding the correct index
        for(int i = std::min(flames.count - 1, flameId); i >= 0; i--)
        {
            Flame& f = flames[i];
            if(f.position.x == x && f.position.y == y)
            {
                // we found the correct flame object
                if(timeStep == f.destroyedWoodAtTimeStep)
                {
                    // the existing flame destroyed some wood block at this timestep, stop here
                    outContinueFlameSpawn = false;
                    outSpawnFlame = false;
                    return;
                }

                if(f.timeLeft == FLAME_LIFETIME)
                {
                    // skip this flame, there already is a flame with the same lifetime
                    outContinueFlameSpawn = true;
                    outSpawnFlame = false;
                    return;
                }

                // the lifetime changes. Due to the ordering, we have to remove the old flame
                if(i == 0)
                {
                    flames.PopElem();
                    flames[0].timeLeft += f.timeLeft;
                }
                else
                {
                    flames[i - 1].timeLeft += f.timeLeft;
                    flames.RemoveAt(i);
                }

                outContinueFlameSpawn = true;
                outSpawnFlame = true;
                return;
            }
        }
    }

    // simply continue spawning flames
    outContinueFlameSpawn = true;
    outSpawnFlame = true;
}

/**
 * @brief SpawnFlameItem Spawns a single flame item on the board
 * @param s The state on which the flames should be spawned
 * @param x The x position of the fire
 * @param y The y position of the fire
 * @param signature An auxiliary integer less than 255
 * @return Should we continue spawing flames in that direction?
 */
bool SpawnFlameItem(State& s, int x, int y, int timeStep, bool isCenterFlame)
{
    int boardItem = s.board[y][x];

    // stop at rigid blocks
    if (boardItem == Item::RIGID)
        return false;

    if(boardItem >= Item::AGENT0)
    {
        s.Kill(boardItem - Item::AGENT0);
    }

    if(!isCenterFlame && (boardItem == Item::BOMB || boardItem >= Item::AGENT0))
    {
        // chain explosions (do not chain self)
        // note: bombs can also be "hidden" below agents
        for(int i = 0; i < s.bombs.count; i++)
        {
            if(BMB_POS(s.bombs[i]) == (x + (y << 4)))
            {
                s.ExplodeBombAt(i);
                return true;
            }
        }
    }

    bool spawnFlame = false, continueSpawn = false;
    _cleanFlameSpawnPosition(s.flames, boardItem, x, y, timeStep, spawnFlame, continueSpawn);

    if(spawnFlame)
    {
        Flame& newFlame = s.flames.NextPos();
        newFlame.position.x = x;
        newFlame.position.y = y;

        // optimization: additive timeLeft in flame queue
        if(isCenterFlame)
        {
            if(s.flames.count == 0)
            {
                newFlame.timeLeft = FLAME_LIFETIME;
            }
            else
            {
                newFlame.timeLeft = FLAME_LIFETIME - s.currentFlameTime;
            }

            s.currentFlameTime = FLAME_LIFETIME;
        }
        else
        {
            newFlame.timeLeft = 0;
        }

        // update the board
        s.board[y][x] = Item::FLAME + (s.flames.count << 3);
        s.flames.count++;

        if(IS_WOOD(boardItem))
        {
            // set the powerup flag
            s.board[y][x] += WOOD_POWFLAG(boardItem);
            // remember that we destroyed wood here
            newFlame.destroyedWoodAtTimeStep = timeStep;
            // stop here, we found wood
            return false;
        }
    }

    return continueSpawn;
}

/**
 * @brief PopBomb A proxy for FixedQueue::PopElem, but also
 * takes care of agent count
 */
inline void PopBomb(State& state)
{
    state.agents[BMB_ID(state.bombs[0])].bombCount--;
    state.bombs.PopElem();
}

/**
 * @brief IsOutOfBounds Checks wether a given position is out of bounds
 */
inline bool IsOutOfBounds(const int& x, const int& y)
{
    return x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE;
}

///////////////////
// State Methods //
///////////////////

void State::ExplodeBombAt(int i)
{
    Bomb b = bombs[i];

    // remove the bomb
    agents[BMB_ID(b)].bombCount--;
    bombs.RemoveAt(i);

    // spawn flames, this may trigger other explosions
    int x = BMB_POS_X(b);
    int y = BMB_POS_Y(b);
    SpawnFlames(x, y, BMB_STRENGTH(b));
}

void State::PlantBomb(int x, int y, int id, bool setItem)
{
    PlantBombModifiedLife(x, y,  id, BOMB_LIFETIME, setItem);
}

void State::PlantBombModifiedLife(int x, int y, int id, int lifeTime, bool setItem)
{
    if(agents[id].bombCount >= agents[id].maxBombCount)
    {
        return;
    }

    Bomb* b = &bombs.NextPos();
    SetBombID(*b, id);
    SetBombPosition(*b, x, y);
    SetBombStrength(*b, agents[id].bombStrength);
    SetBombDirection(*b, Direction::IDLE);
    SetBombMovedFlag(*b, false);
    SetBombTime(*b, lifeTime);

    if(setItem)
    {
        board[y][x] = Item::BOMB;
    }

    agents[id].bombCount++;
    bombs.count++;
}

void State::PopFlames()
{
    while (flames.count > 0 && flames[0].timeLeft <= 0) {
        Flame& f = flames[0];
        // get the item behind the flame (can be 0 for passage)
        bboard::Item newItem = FlagItem(FLAME_POWFLAG(board[f.position.y][f.position.x]));
        // remove the flame
        board[f.position.y][f.position.x] = newItem;
        flames.PopElem();
    }
}

Item State::FlagItem(int pwp)
{
    switch (pwp) {
        case 1: return Item::EXTRABOMB;
        case 2: return Item::INCRRANGE;
        case 3: return Item::KICK;
        default: return Item::PASSAGE;
    }
}

int State::ItemFlag(Item item)
{
    switch (item) {
        case Item::EXTRABOMB: return 1;
        case Item::INCRRANGE: return 2;
        case Item::KICK: return 3;
        default: return 0;
    }
}

void State::ExplodeTopBomb()
{
    Bomb& c = bombs[0];
    SpawnFlames(BMB_POS_X(c), BMB_POS_Y(c), BMB_STRENGTH(c));
    PopBomb(*this);
}

void State::SpawnFlames(int x, int y, int strength)
{    
    // spawn flame in center
    if(!SpawnFlameItem(*this, x, y, timeStep, true))
    {
        return;
    }

    // spawn subflames

    // right
    for(int i = 1; i <= strength; i++)
    {
        if(x + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x + i, y, timeStep, false))
        {
            break;
        }
    }

    // left
    for(int i = 1; i <= strength; i++)
    {
        if(x - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x - i, y, timeStep, false))
        {
            break;
        }
    }

    // top
    for(int i = 1; i <= strength; i++)
    {
        if(y + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x, y + i, timeStep, false))
        {
            break;
        }
    }

    // bottom
    for(int i = 1; i <= strength; i++)
    {
        if(y - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x, y - i, timeStep, false))
        {
            break;
        }
    }
}

bool State::HasBomb(int x, int y)
{
    for(int i = 0; i < bombs.count; i++)
    {
        if(BMB_POS_X(bombs[i]) == x && BMB_POS_Y(bombs[i]) == y)
        {
            return true;
        }
    }
    return false;
}

Bomb* State::GetBomb(int x, int y)
{
    for(int i = 0; i < bombs.count; i++)
    {
        if(BMB_POS_X(bombs[i]) == x && BMB_POS_Y(bombs[i]) == y)
        {
            return &bombs[i];
        }
    }
    return nullptr;
}

int State::GetAgent(int x, int y)
{
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        if(!agents[i].dead && agents[i].x == x && agents[i].y == y)
        {
            return i;
        }
    }
    return -1;
}

int State::GetBombIndex(int x, int y)
{
    for(int i = 0; i < bombs.count; i++)
    {
        if(BMB_POS_X(bombs[i]) == x && BMB_POS_Y(bombs[i]) == y)
        {
            return i;
        }
    }
    return -1;
}

void State::PutAgent(int x, int y, int agentID)
{
    board[y][x] = Item::AGENT0 + agentID;

    agents[agentID].x = x;
    agents[agentID].y = y;
}

void State::PutAgentsInCorners(int a0, int a1, int a2, int a3, int padding)
{
    const int min = padding;
    const int max = BOARD_SIZE - (1 + padding);

    PutAgent(min, min, a0);
    PutAgent(max, min, a1);
    PutAgent(max, max, a2);
    PutAgent(min, max, a3);
}

//////////////////////
// bboard namespace //
//////////////////////

inline int _invert(const int boardPos)
{
    return BOARD_SIZE - 1 - boardPos;
}

/**
 * Selects a random element arr[i] with i in range [0, count - 1]
 * and swaps arr[i] with arr[0]. This means arr[1:count-1] will
 * contain the elements which were not returned yet. Repeated use
 * with incremented arr start pointer allows for random and unique
 * in-place selection.
 *
 * @tparam preserveElements Can be disabled to save a copy operation,
 * arr[0] will not be set correctly. Only use this when you no longer
 * need the elements after they were selected.
 * @tparam T Type of the array.
 * @tparam Type of the random number generator.
 *
 * @param arr A pointer to the start of the array.
 * @param count The number of remaining elements.
 * @param rng A number generator. Has to return values >= 0.
 * @return A random element between arr[0] and arr[count - 1].
 */
template<bool preserveElements=true, typename T, typename RNG>
inline T _selectRandomInPlace(T* arr, int count, RNG& rng)
{
    // get a random index in [0, count - 1]
    const int index = rng() % count;

    // select value
    const T b = *(arr + index);

    // remember the value at arr[0] which we did not select
    *(arr + index) = *arr;
    if (preserveElements)
    {
        // do a regular swap if we want to preserve all elements in arr
        *arr = b;
    }

    // return the selected element
    return b;
}

void InitBoard(State& state, long seed, bool randomAgentPositions, int numRigid, int numWood, int numPowerUps, int padding, int breathingRoomSize)
{
    std::mt19937 rng(seed);

    // initialize everything as passages
    std::fill_n(&state.board[0][0], BOARD_SIZE * BOARD_SIZE, (int)Item::PASSAGE);

    // insert agents at their respective positions
    std::array<int, 4> f = {0, 1, 2, 3};
    if(randomAgentPositions)
    {
        std::shuffle(f.begin(), f.end(), rng);
    }
    state.PutAgentsInCorners(f[0], f[1], f[2], f[3], padding);

    // create the board

    std::vector<Position> woodCoordinates;
    woodCoordinates.reserve(numWood);

    std::vector<Position> coordinates;
    coordinates.reserve(BOARD_SIZE * (BOARD_SIZE - 4) + 8 * padding + 4);

    // create a "breathing room" around agents of length freeSpaceUntil
    // and place wooden boxes to form passages between them.
    // The board will look like this (with padding to walls):
    //            |   padding    |
    // padding - [1][ ][x][x][ ][2] - padding
    //           [ ][?]      [?][ ] <- this is the breathing room
    //           [x]   [?][?]   [x]    (vertical and horizontal space)
    //           [x]   [?][?]   [x] <- these are the wooden boxes separating
    //           [ ][?]      [?][ ]    the players' breathing rooms
    // padding - [4][ ][x][x][ ][3] - padding
    //            |   padding    |

    int tmpNorm = -1;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int  j = 0; j < BOARD_SIZE; j++)
        {
            if (i == padding || _invert(i) == padding) {
                tmpNorm = std::min(j, _invert(j));
            }
            else if (j == padding || _invert(j) == padding) {
                tmpNorm = std::min(i, _invert(i));
            }

            if(tmpNorm != -1) {
                // breathing room
                if (tmpNorm >= padding && tmpNorm <= breathingRoomSize) {
                    tmpNorm = -1;
                    continue;
                }
                // wooden boxes
                else if (tmpNorm > padding) {
                    tmpNorm = -1;
                    state.board[i][j] = Item::WOOD;
                    woodCoordinates.push_back((Position) {i, j});
                    numWood--;
                    continue;
                }
            }

            // remember this coordinate, we can randomly add stuff later
            coordinates.push_back((Position) {i, j});
        }
    }

    int i = 0;
    // create rigid walls
    while (numRigid > 0) {
        // select random coordinate
        Position coord = _selectRandomInPlace<false>(coordinates.data() + i, coordinates.size() - i, rng);
        i++;

        // create wall
        state.board[coord.y][coord.x] = Item::RIGID;
        numRigid--;
    }

    // create wooden blocks (keep index)
    while (numWood > 0) {
        Position coord = _selectRandomInPlace<false>(coordinates.data() + i, coordinates.size() - i, rng);
        i++;

        state.board[coord.y][coord.x] = Item::WOOD;
        woodCoordinates.push_back(coord);
        numWood--;
    }

    i = 0;
    std::uniform_int_distribution<int> choosePwp(1, 3);
    // insert items
    while (numPowerUps > 0) {
        Position coord = _selectRandomInPlace<false>(woodCoordinates.data() + i, woodCoordinates.size() - i, rng);
        i++;

        state.board[coord.y][coord.x] = Item::WOOD + choosePwp(rng);
        numPowerUps--;
    }
}

void StartGame(State* state, Agent* agents[AGENT_COUNT], int timeSteps)
{
    Move moves[4];

    for(int i = 0; i < timeSteps; i++)
    {
        std::cout << "\033c"; // clear console on linux
        for(int j = 0; j < AGENT_COUNT; j++)
        {
            moves[j] = agents[j]->act(state);
        }

        Step(state, moves);
        PrintState(state);

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void PrintState(State* state, bool clearConsole)
{
    std::string result = "";

    // clears console on linux
    if(clearConsole)
        std::cout << "\033c";

    for(int y = 0; y < BOARD_SIZE; y++)
    {
        for(int x = 0; x < BOARD_SIZE; x++)
        {
            int item = state->board[y][x];
            result += PrintItem(item);
        }
        std::cout << (result) << "          ";
        result = "";
        // Print AgentInfo
        if(y < AGENT_COUNT)
        {
            std::cout << "Agent " << y << ": ";
            std::cout << PrintItem(Item::EXTRABOMB) << ": " << state->agents[y].maxBombCount << " ";
            std::cout << PrintItem(Item::INCRRANGE) << ": " << state->agents[y].bombStrength << " ";
            std::cout << PrintItem(Item::KICK) << ": " << state->agents[y].canKick << " ";
        }
        else if(y == AGENT_COUNT + 1)
        {
            std::cout << "Bombs:  [  ";
            for(int i = 0; i < state->bombs.count; i++)
            {
                std::cout << BMB_ID(state->bombs[i]) << "  ";
            }
            std::cout << "]";
        }
        else if(y == AGENT_COUNT + 2)
        {
            std::cout << "Flames: [  ";
            int cumulativeTime = 0;
            for(int i = 0; i < state->flames.count; i++)
            {
                if(state->flames[i].timeLeft != 0)
                {
                    cumulativeTime += state->flames[i].timeLeft;
                    std::cout << cumulativeTime << "  ";
                }
            }
            std::cout << "]";
        }
        std::cout << std::endl;
    }
}

std::string PrintItem(int item)
{
    std::string wood = "[\u25A0]";
    std::string fire = " \U0000263C ";

    switch(item)
    {
        case Item::PASSAGE:
            return "   ";
        case Item::RIGID:
            return "[X]";
        case Item::BOMB:
            return " \u25CF ";
        case Item::EXTRABOMB:
            return " \u24B7 ";
        case Item::INCRRANGE:
            return " \u24C7 ";
        case Item::KICK:
            return " \u24C0 ";
    }
    if(IS_WOOD(item))
    {
        return FBLU(wood);
    }
    if(IS_FLAME(item))
    {
        return FRED(fire);
    }
    //agent number
    if(item >= Item::AGENT0)
    {
        return " "  +  std::to_string(item - Item::AGENT0) + " ";
    }
    else
    {
        return "[?]";
    }
}

}

