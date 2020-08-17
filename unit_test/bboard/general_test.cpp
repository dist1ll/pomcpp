#include <iostream>

#include "catch.hpp"
#include "bboard.hpp"

using namespace bboard;

void TestQueue(FixedQueue<Bomb, 10>& queue)
{
    for(int i = 0; i < 10; i++)
    {
        int& current = queue.NextPos();
        current = i;
        queue.count++;
    }

    REQUIRE(queue.count == 10);

    queue.PopElem();
    queue.PopElem();
    queue.PopElem();

    REQUIRE(queue.count == 7);
    REQUIRE(queue[0] == 3);

    queue.RemoveAt(5);

    REQUIRE(queue.count == 6);
    REQUIRE(queue[4] == 7);
    REQUIRE(queue[5] == 9);

    queue.RemoveAt(0);

    REQUIRE(queue[0] == 4);

    queue.RemoveAt(4);

    REQUIRE(queue.count == 4);
    REQUIRE(queue[3] == 7);
}

TEST_CASE("Fixed Size Queue", "[general]")
{
    FixedQueue<Bomb, 10> queue;
    SECTION("Index 0")
    {
        queue.index = 0;
        TestQueue(queue);
    }
    SECTION("Index 5")
    {
        queue.index = 5;
        TestQueue(queue);
    }
    SECTION("Index 2")
    {
        queue.index = 2;
        TestQueue(queue);
    }
}
