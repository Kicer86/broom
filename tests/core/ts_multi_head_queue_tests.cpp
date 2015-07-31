
#include <sstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <core/ts_multi_head_queue.hpp>


TEST(TS_MultiHeadQueueTest, isConstructible)
{
    EXPECT_NO_THROW(
        {
            TS_MultiHeadQueue<int> q;
        });
}


TEST(TS_MultiHeadQueueTest, QuitsWithTimeoutWhenNoDataIncomes)
{
    TS_MultiHeadQueue<int> q;
    using namespace std::literals;

    auto r = q.pop_for(1ms);

    EXPECT_EQ(false, r.is_initialized());
}


TEST(TS_MultiHeadQueueTest, ReturnsWhatProducerGenerated)
{
    TS_MultiHeadQueue<int> q;
    using namespace std::literals;

    auto p = q.prepareProducer();

    p->push(1);
    p->push(2);
    p->push(3);
    p->push(4);

    auto r = q.pop();
    EXPECT_EQ(1, *r);

    r = q.pop();
    EXPECT_EQ(2, *r);

    r = q.pop();
    EXPECT_EQ(3, *r);

    r = q.pop();
    EXPECT_EQ(4, *r);
}


TEST(TS_MultiHeadQueueTest, ReturnsMixedProductionOfManyProducers)
{
    TS_MultiHeadQueue<int> q;
    using namespace std::literals;

    auto p1 = q.prepareProducer();
    auto p2 = q.prepareProducer();
    auto p3 = q.prepareProducer();
    auto p4 = q.prepareProducer();

    p1->push(101);
    p1->push(102);
    p1->push(103);
    p1->push(104);

    p2->push(201);
    p2->push(202);
    p2->push(203);
    p2->push(204);

    p3->push(301);
    p3->push(302);
    p3->push(303);
    p3->push(304);

    p4->push(401);
    p4->push(402);
    p4->push(403);
    p4->push(404);

    std::set<int> expecting;
    int sum = 0;

    for(int i = 0; i < 16; i++)
    {
        if (i % 4 == 0)
            expecting = {1, 2, 3, 4};

        auto r = q.pop();

        const int p = (*r)/100;          // producer id
        auto it = expecting.find(p);

        EXPECT_NE(expecting.end(), it);  // we should be expecting this producer

        expecting.erase(it);             // remove this producer from expecting ones

        sum += (*r);
    }
}


