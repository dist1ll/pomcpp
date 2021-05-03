#include "bboard.hpp"
#include "agents.hpp"
#include "strategy.hpp"
#include "step_utility.hpp"

using namespace bboard;
using namespace bboard::strategy;

namespace agents
{

void _init_axes(SimpleUnbiasedAgent& me)
{
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        me.agentAxis[i] = i;
    }

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        me.boardAxisX[i] = i;
        me.boardAxisY[i] = i;
    }

    me.dirAxis[0] = Move::UP;
    me.dirAxis[1] = Move::DOWN;
    me.dirAxis[2] = Move::LEFT;
    me.dirAxis[3] = Move::RIGHT;
}

void _shuffle_axes(SimpleUnbiasedAgent& me)
{
    // randomize axis order
    std::shuffle(me.agentAxis.begin(), me.agentAxis.end(), me.rng);
    std::shuffle(me.boardAxisX.begin(), me.boardAxisX.end(), me.rng);
    std::shuffle(me.boardAxisY.begin(), me.boardAxisY.end(), me.rng);
    std::shuffle(me.dirAxis.begin(), me.dirAxis.end(), me.rng);
}

SimpleUnbiasedAgent::SimpleUnbiasedAgent()
{
    std::random_device rd;  // non explicit seed
    rng = std::mt19937_64(rd());
    _init_axes(*this);
    reset();
}

SimpleUnbiasedAgent::SimpleUnbiasedAgent(long seed)
{
    rng = std::mt19937_64(seed);
    _init_axes(*this);
    reset();
}

void SimpleUnbiasedAgent::reset()
{
    // reset the internal state
    moveQueue.count = 0;
    recentPositions.count = 0;

    _shuffle_axes(*this);
}

// TODO: not modified
bool _UHasRPLoop(SimpleAgent& me)
{
    for(int i = 0; i < me.recentPositions.count / 2; i++)
    {
        if(!(me.recentPositions[i] == me.recentPositions[i + 2]))
        {
            return false;
        }
    }

    return true;
}

// not modified
bool _UCheckPos(const State& state, int x, int y)
{
    return !util::IsOutOfBounds(x, y) && IS_WALKABLE(state.items[y][x]);
}

void _USafeDirections(const SimpleUnbiasedAgent& me, const State& state, FixedQueue<Move, MOVE_COUNT>& q, int x, int y)
{
    for (Move m : me.dirAxis) 
    {
        Position desired = util::DesiredPosition(x, y, m);
        int d = IsInDanger(state, desired.x, desired.y);
        if(_UCheckPos(state, desired.x, desired.y) && _safe_condition(d))
        {
            q.AddElem(m);
        }
    }
}

Move _UMoveSafeOneSpace(SimpleUnbiasedAgent& me, const State* state)
{
    const AgentInfo& a = state->agents[me.id];
    me.moveQueue.count = 0;
    
    std::shuffle(me.dirAxis.begin(), me.dirAxis.end(), me.rng);
    _USafeDirections(me, *state, me.moveQueue, a.x, a.y);
    SortDirections(me.moveQueue, me.recentPositions, a.x, a.y);

    if(me.moveQueue.count == 0)
        return Move::IDLE;
    else
        return me.moveQueue[me.rng() % std::min(2, me.moveQueue.count)];
}

Move _UMoveTowardsSafePlace(const SimpleUnbiasedAgent &me, const State& state, const RMap& r, int radius)
{
    int originX = r.source.x;
    int originY = r.source.y;

    for(int y : me.boardAxisY)
    {
        for(int x : me.boardAxisX)
        {
            if(y >= radius || y < originY - radius)
                break;

            if(x >= radius || x < originX - radius)
                break;

            if(util::IsOutOfBounds({x, y}) ||
                    std::abs(x - originX) + std::abs(y - originY) > radius) continue;

            if(r.GetDistance(x, y) != 0 && _safe_condition(IsInDanger(state, x, y)))
            {
                return MoveTowardsPosition(r, {x, y});
            }
        }
    }
    return Move::IDLE;
}

Move _UMoveTowardsEnemy(const SimpleUnbiasedAgent &me, const State& state, const RMap& r, int radius)
{
    const Position& a = r.source;

    for(int i : me.agentAxis)
    {
        const AgentInfo& inf = state.agents[i];

        if((inf.x == a.x && inf.y == a.y) || inf.dead) continue;

        int x = state.agents[i].x;
        int y = state.agents[i].y;
        if(std::abs(x - a.x) + std::abs(y - a.y) > radius)
        {
            continue;
        }
        else
        {
            return MoveTowardsPosition(r, {x, y});
        }

    }
    return Move::IDLE;
}

Move _UDecide(SimpleUnbiasedAgent& me, const State* state)
{
    const AgentInfo& a = state->agents[me.id];
    FillRMap(*state, me.r, me.id);

    me.danger = IsInDanger(*state, me.id);

    if(me.danger > 0) // ignore danger if not too high
    {
        Move m = _UMoveTowardsSafePlace(me, *state, me.r, me.danger);
        Position p = util::DesiredPosition(a.x, a.y, m);
        if(!util::IsOutOfBounds(p.x, p.y) && IS_WALKABLE(state->items[p.y][p.x]) &&
                _safe_condition(IsInDanger(*state, p.x, p.y), 2))
        {
            return m;
        }
    }
    else if(a.bombCount < a.maxBombCount)
    {
        //prioritize enemy destruction
        if(IsAdjacentEnemy(*state, me.id, 1))
        {
            return Move::BOMB;
        }

        if(IsAdjacentEnemy(*state, me.id, 7))
        {
            // if you're stuck in a loop try to break out by randomly selecting
            // an action ( we could IDLE but the mirroring of agents is tricky)
            if(_UHasRPLoop(me)) {
                return Move(me.rng() % 5);
            }

            Move m = _UMoveTowardsEnemy(me, *state, me.r, 7);
            Position p = util::DesiredPosition(a.x, a.y, m);
            if(!util::IsOutOfBounds(p.x, p.y) && IS_WALKABLE(state->items[p.y][p.x]) &&
                    _safe_condition(IsInDanger(*state, p.x, p.y), 5))
            {
                return m;
            }
        }

        if(IsAdjacentItem(*state, me.id, 1, Item::WOOD))
        {
            return Move::BOMB;
        }
    }

    // TODO: Collect powerups

    return _UMoveSafeOneSpace(me, state);
}

Move SimpleUnbiasedAgent::act(const State* state)
{
    const AgentInfo& a = state->agents[id];
    Move m = _UDecide(*this, state);
    Position p = util::DesiredPosition(a.x, a.y, m);

    if(recentPositions.RemainingCapacity() == 0)
    {
        recentPositions.PopElem();
    }
    recentPositions.AddElem(p);

    return m;
}

}
