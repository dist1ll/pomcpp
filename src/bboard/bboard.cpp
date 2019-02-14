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
 * @brief SpawnFlameItem Spawns a single flame item on the board
 * @param s The state on which the flames should be spawned
 * @param x The x position of the fire
 * @param y The y position of the fire
 * @param signature An auxiliary integer less than 255
 * @return Could the flame be spawned?
 */
bool SpawnFlameItem(State& s, int x, int y, uint16_t signature = 0)
{
    if(s.board[y][x] >= Item::AGENT0)
    {
        s.Kill(s.board[y][x] - Item::AGENT0);
    }
    if(s.board[y][x] == Item::BOMB || s.board[y][x] >= Item::AGENT0)
    {
        for(int i = 0; i < s.bombs.count; i++)
        {
            if(BMB_POS(s.bombs[i]) == (x + (y << 4)))
            {
                s.ExplodeBombAt(i);
                break;
            }
        }
    }

    if(s.board[y][x] != Item::RIGID)
    {
        int old = s.board[y][x];
        bool wasWood = IS_WOOD(old);
        s.board[y][x] = Item::FLAMES + signature;
        if(wasWood)
        {
            s.board[y][x]+= WOOD_POWFLAG(old); // set the powerup flag
        }
        return !wasWood; // if wood, then only destroy 1
    }
    else
    {
        return false;
    }
}

int ChooseItemOuter(int tmp)
{
    if(tmp > 2 || tmp == 0)
    {
        return Item::PASSAGE;
    }
    else if(tmp == 2)
    {
        return Item::WOOD;
    }
    else if(tmp == 1)
    {
        return Item::RIGID;
    }
    return Item::PASSAGE;
}

int ChooseItemInner(int tmp)
{
    if(tmp >= 2)
    {
        return Item::WOOD;
    }
    else if(tmp == 1)
    {
        return Item::RIGID;
    }
    return Item::PASSAGE;
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
    int x = BMB_POS_X(bombs[i]);
    int y = BMB_POS_Y(bombs[i]);
    SpawnFlame(x, y, agents[BMB_ID(bombs[i])].bombStrength);
    agents[BMB_ID(bombs[i])].bombCount--;
    bombs.RemoveAt(i);
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
    // TODO: velocity
    SetBombTime(*b, lifeTime);

    if(setItem)
    {
        board[y][x] = Item::BOMB;
    }

    agents[id].bombCount++;
    bombs.count++;
}

void State::PopFlame()
{
    Flame& f = flames[0];
    const int s = f.strength;
    int x = f.position.x;
    int y = f.position.y;

    uint16_t signature = uint16_t(x + BOARD_SIZE * y);

    // iterate over both axis (from x-s to x+s // y-s to y+s)
    for(int i = -s; i <= s; i++)
    {
        if(!IsOutOfBounds(x + i, y) && IS_FLAME(board[y][x + i]))
        {
            // only remove if this is my own flame
            int b = board[y][x + i];
            if(FLAME_ID(b) == signature)
            {
                board[y][x + i] = FlagItem(FLAME_POWFLAG(b));
            }
        }
        if(!IsOutOfBounds(x, y + i) && IS_FLAME(board[y + i][x]))
        {
            int b = board[y + i][x];
            if(FLAME_ID(b) == signature)
            {
                board[y + i][x] = FlagItem(FLAME_POWFLAG(b));
            }
        }
    }

    flames.PopElem();
}

Item State::FlagItem(int pwp)
{
    if     (pwp == 0) return Item::PASSAGE;
    else if(pwp == 1) return Item::EXTRABOMB;
    else if(pwp == 2) return Item::INCRRANGE;
    else if(pwp == 3) return Item::KICK;
    else              return Item::PASSAGE;
}

void State::ExplodeTopBomb()
{
    Bomb& c = bombs[0];
    SpawnFlame(BMB_POS_X(c), BMB_POS_Y(c), BMB_STRENGTH(c));
    PopBomb(*this);
}

void State::SpawnFlame(int x, int y, int strength)
{
    Flame& f = flames.NextPos();
    f.position.x = x;
    f.position.y = y;
    f.strength = strength;
    f.timeLeft = FLAME_LIFETIME;

    // unique flame id
    uint16_t signature = uint16_t((x + BOARD_SIZE * y) << 3);

    flames.count++;

    // kill agent possibly in origin
    if(board[y][x] >= Item::AGENT0)
    {
        Kill(board[y][x] - Item::AGENT0);
    }

    // override origin
    board[y][x] = Item::FLAMES + signature;

    // right
    for(int i = 1; i <= strength; i++)
    {
        if(x + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x + i, y, signature))
        {
            break;
        }
    }

    // left
    for(int i = 1; i <= strength; i++)
    {
        if(x - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x - i, y, signature))
        {
            break;
        }
    }

    // top
    for(int i = 1; i <= strength; i++)
    {
        if(y + i >= BOARD_SIZE) break; // bounds

        if(!SpawnFlameItem(*this, x, y + i, signature))
        {
            break;
        }
    }

    // bottom
    for(int i = 1; i <= strength; i++)
    {
        if(y - i < 0) break; // bounds

        if(!SpawnFlameItem(*this, x, y - i, signature))
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
        if(agents[i].x == x && agents[i].y == y)
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
    int b = Item::AGENT0 + agentID;
    board[y][x] = b;

    agents[agentID].x = x;
    agents[agentID].y = y;
}

void State::PutAgentsInCorners(int a0, int a1, int a2, int a3)
{
    int b = Item::AGENT0;

    board[0][0] = b + a0;
    board[0][BOARD_SIZE - 1] = b + a1;
    board[BOARD_SIZE - 1][BOARD_SIZE - 1] = b + a2;
    board[BOARD_SIZE - 1][0] = b + a3;

    agents[a1].x = agents[a2].x = BOARD_SIZE - 1;
    agents[a2].y = agents[a3].y = BOARD_SIZE - 1;
}

//////////////////////
// bboard namespace //
//////////////////////

void InitState(State* result, int a0, int a1, int a2, int a3)
{
    // Randomly put obstacles
    InitBoardItems(*result);
    result->PutAgentsInCorners(a0, a1, a2, a3);
}

void InitBoardItems(State& result, int seed)
{
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> intDist(0,6);

    FixedQueue<int, BOARD_SIZE * BOARD_SIZE> q;

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int  j = 0; j < BOARD_SIZE; j++)
        {
            int tmp = intDist(rng);
            result.board[i][j] = ChooseItemOuter(tmp);

            if(IS_WOOD(result.board[i][j]))
            {
                q.AddElem(j + BOARD_SIZE * i);
            }
        }
    }

    std::uniform_int_distribution<int> idxSample(0, q.count);
    std::uniform_int_distribution<int> choosePwp(1, 4);
    int total = 0;
    while(true)
    {
        int idx = q[idxSample(rng)];
        if((result.board[0][idx] & 0xFF) == 0)
        {
            result.board[0][idx] += choosePwp(rng);
            total++;
        }

        if(total >= float(q.count)/2)
            break;
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
            int i = y;
            std::printf("Agent %d: %s %d  %s %d  %s %d",
                        i,
                        PrintItem(Item::EXTRABOMB).c_str(),state->agents[i].maxBombCount,
                        PrintItem(Item::INCRRANGE).c_str(),state->agents[i].bombStrength,
                        PrintItem(Item::KICK).c_str(),state->agents[i].canKick);
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
            for(int i = 0; i < state->flames.count; i++)
            {
                std::cout << state->flames[i].timeLeft << "  ";
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

