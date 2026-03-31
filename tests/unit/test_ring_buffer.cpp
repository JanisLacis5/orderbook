#include <gtest/gtest.h>

#include <span>
#include <stdexcept>
#include <utility>

#include "ring_buffer.h"

using testType = int;

class RingBufferTest : public testing::Test {
public:
    size_t cap = 4u;
    ring_buffer<testType> rb{cap};
};

TEST_F(RingBufferTest, properties)
{
    EXPECT_FALSE(std::is_default_constructible_v<ring_buffer<testType>>);
    EXPECT_TRUE((std::is_constructible_v<ring_buffer<testType>, unsigned long>));
    EXPECT_TRUE((std::is_constructible_v<ring_buffer<testType>, unsigned long, std::allocator<testType>>));
    EXPECT_FALSE(std::is_copy_constructible_v<ring_buffer<testType>>);
    EXPECT_TRUE(std::is_move_constructible_v<ring_buffer<testType>>);
    EXPECT_FALSE(std::is_copy_assignable_v<ring_buffer<testType>>);
    EXPECT_TRUE(std::is_move_assignable_v<ring_buffer<testType>>);
    EXPECT_TRUE(std::is_destructible_v<ring_buffer<testType>>);
}

TEST_F(RingBufferTest, ConstructorThrowsOnZeroCapacity)
{
    EXPECT_THROW((ring_buffer<int>{0}), std::logic_error);
}

TEST_F(RingBufferTest, NewBufferStartsEmpty)
{
    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 0);
    EXPECT_EQ(rb.capacity(), cap);
}

TEST_F(RingBufferTest, PushBackLvalueIncreasesSize)
{
    int x = 7;

    EXPECT_TRUE(rb.push_back(x));
    EXPECT_EQ(rb.size(), 1);
    EXPECT_FALSE(rb.empty());
    EXPECT_EQ(rb.front(), 7);
    EXPECT_EQ(rb.back(), 7);
}

TEST_F(RingBufferTest, PushBackRvalueIncreasesSize)
{
    EXPECT_TRUE(rb.push_back(9));
    EXPECT_EQ(rb.size(), 1);
    EXPECT_EQ(rb.front(), 9);
    EXPECT_EQ(rb.back(), 9);
}

TEST_F(RingBufferTest, FillToCapacityMakesBufferFull)
{
    EXPECT_TRUE(rb.push_back(1));
    EXPECT_TRUE(rb.push_back(2));
    EXPECT_TRUE(rb.push_back(3));
    EXPECT_TRUE(rb.push_back(4));

    EXPECT_TRUE(rb.full());
    EXPECT_EQ(rb.size(), cap);
}

TEST_F(RingBufferTest, PushBackFailsWhenFull)
{
    EXPECT_TRUE(rb.push_back(1));
    EXPECT_TRUE(rb.push_back(2));
    EXPECT_TRUE(rb.push_back(3));
    EXPECT_TRUE(rb.push_back(4));
    EXPECT_FALSE(rb.push_back(5));

    EXPECT_EQ(rb.size(), cap);
}

TEST_F(RingBufferTest, PopFrontOutReturnsElementsInFifoOrder)
{
    int out = 0;

    rb.push_back(10);
    rb.push_back(20);
    rb.push_back(30);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 10);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 20);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 30);

    EXPECT_TRUE(rb.empty());
}

TEST_F(RingBufferTest, PopBackOutReturnsElementsInLifoOrder)
{
    int out = 0;

    rb.push_back(10);
    rb.push_back(20);
    rb.push_back(30);

    ASSERT_TRUE(rb.pop_back(out));
    EXPECT_EQ(out, 30);

    ASSERT_TRUE(rb.pop_back(out));
    EXPECT_EQ(out, 20);

    ASSERT_TRUE(rb.pop_back(out));
    EXPECT_EQ(out, 10);

    EXPECT_TRUE(rb.empty());
}

TEST_F(RingBufferTest, PopFrontOptionalReturnsValue)
{
    rb.push_back(11);

    auto out = rb.pop_front();
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(*out, 11);
    EXPECT_TRUE(rb.empty());
}

TEST_F(RingBufferTest, PopBackOptionalReturnsValue)
{
    rb.push_back(17);

    auto out = rb.pop_back();
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out.value(), 17);
    EXPECT_TRUE(rb.empty());
}

TEST_F(RingBufferTest, PopFrontOutFailsOnEmptyBuffer)
{
    int out = 123;

    EXPECT_FALSE(rb.pop_front(out));
    EXPECT_EQ(out, 123);
}

TEST_F(RingBufferTest, PopBackOutFailsOnEmptyBuffer)
{
    int out = 123;

    EXPECT_FALSE(rb.pop_back(out));
    EXPECT_EQ(out, 123);
}

TEST_F(RingBufferTest, PopFrontOptionalOnEmptyReturnsNullopt)
{
    EXPECT_FALSE(rb.pop_front().has_value());
}

TEST_F(RingBufferTest, PopBackOptionalOnEmptyReturnsNullopt)
{
    EXPECT_FALSE(rb.pop_back().has_value());
}

TEST_F(RingBufferTest, ConsumeFrontZeroDoesNothing)
{
    rb.push_back(1);
    rb.push_back(2);

    EXPECT_TRUE(rb.consume_front(0));
    EXPECT_EQ(rb.size(), 2);
    EXPECT_EQ(rb.front(), 1);
}

TEST_F(RingBufferTest, ConsumeBackZeroDoesNothing)
{
    rb.push_back(1);
    rb.push_back(2);

    EXPECT_TRUE(rb.consume_back(0));
    EXPECT_EQ(rb.size(), 2);
    EXPECT_EQ(rb.front(), 1);
}

TEST_F(RingBufferTest, ConsumeFrontRemovesRequestedCount)
{
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    rb.push_back(4);

    EXPECT_TRUE(rb.consume_front(2));
    EXPECT_EQ(rb.size(), 2);
    EXPECT_EQ(rb.front(), 3);
}

TEST_F(RingBufferTest, ConsumeBackRemovesRequestedCount)
{
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    rb.push_back(4);

    EXPECT_TRUE(rb.consume_back(2));
    EXPECT_EQ(rb.size(), 2);

    int out = 0;
    ASSERT_TRUE(rb.pop_back(out));
    EXPECT_EQ(out, 2);

    ASSERT_TRUE(rb.pop_back(out));
    EXPECT_EQ(out, 1);
}

TEST_F(RingBufferTest, ConsumeFrontFailsIfCountExceedsSize)
{
    rb.push_back(1);
    rb.push_back(2);

    EXPECT_FALSE(rb.consume_front(3));
    EXPECT_EQ(rb.size(), 2);
}

TEST_F(RingBufferTest, ConsumeBackFailsIfCountExceedsSize)
{
    rb.push_back(1);
    rb.push_back(2);

    EXPECT_FALSE(rb.consume_back(3));
    EXPECT_EQ(rb.size(), 2);
}

TEST_F(RingBufferTest, WrapAroundPreservesFifoOrder)
{
    int out = 0;

    ASSERT_TRUE(rb.push_back(1));
    ASSERT_TRUE(rb.push_back(2));
    ASSERT_TRUE(rb.push_back(3));

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 1);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 2);

    ASSERT_TRUE(rb.push_back(4));
    ASSERT_TRUE(rb.push_back(5));

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 3);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 4);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 5);

    EXPECT_TRUE(rb.empty());
}

TEST_F(RingBufferTest, MoveConstructorTransfersContents)
{
    ring_buffer<int> src{4};
    src.push_back(1);
    src.push_back(2);

    ring_buffer<int> dst{std::move(src)};

    EXPECT_EQ(dst.size(), 2);

    int out = 0;
    ASSERT_TRUE(dst.pop_front(out));
    EXPECT_EQ(out, 1);
    ASSERT_TRUE(dst.pop_front(out));
    EXPECT_EQ(out, 2);
}

TEST_F(RingBufferTest, MoveAssignmentTransfersContents)
{
    ring_buffer<int> src{4};
    src.push_back(5);
    src.push_back(6);

    ring_buffer<int> dst{3};
    dst.push_back(100);
    dst.push_back(200);

    dst = std::move(src);

    EXPECT_EQ(dst.size(), 2);

    int out = 0;
    ASSERT_TRUE(dst.pop_front(out));
    EXPECT_EQ(out, 5);
    ASSERT_TRUE(dst.pop_front(out));
    EXPECT_EQ(out, 6);
}

TEST_F(RingBufferTest, FrontReturnsOldestElement)
{
    rb.push_back(10);
    rb.push_back(20);
    rb.push_back(30);

    EXPECT_EQ(rb.front(), 10);
}

TEST_F(RingBufferTest, BackReturnsNewestElement)
{
    rb.push_back(10);
    rb.push_back(20);
    rb.push_back(30);

    EXPECT_EQ(rb.back(), 30);
}

TEST_F(RingBufferTest, BackAfterWrapAroundReturnsNewestElement)
{
    int out = 0;

    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 1);

    ASSERT_TRUE(rb.push_back(4));

    EXPECT_EQ(rb.back(), 4);
}

TEST_F(RingBufferTest, ReadableContiguousOnSimpleNonWrappedBufferContainsAllReadableElements)
{
    rb.push_back(10);
    rb.push_back(20);
    rb.push_back(30);

    auto span = rb.readable_contiguous();

    ASSERT_EQ(span.size(), 3);
    EXPECT_EQ(span[0], 10);
    EXPECT_EQ(span[1], 20);
    EXPECT_EQ(span[2], 30);
}

TEST_F(RingBufferTest, ReadableContiguousOnWrappedBufferStartsAtHeadAndContainsFirstReadableBlock)
{
    int out = 0;

    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 1);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 2);

    ASSERT_TRUE(rb.push_back(4));
    ASSERT_TRUE(rb.push_back(5));

    auto span = rb.readable_contiguous();

    ASSERT_EQ(span.size(), 2);
    EXPECT_EQ(span[0], 3);
    EXPECT_EQ(span[1], 4);
}

TEST_F(RingBufferTest, WritableContiguousOnEmptyBufferCoversEntireCapacity)
{
    auto span = rb.writable_contiguous();
    EXPECT_EQ(span.size(), cap);
}

TEST_F(RingBufferTest, WritableContiguousAfterSimplePushesStartsAtTailAndMatchesRemainingCapacity)
{
    ring_buffer<int> rb{5};
    rb.push_back(1);
    rb.push_back(2);

    auto span = rb.writable_contiguous();
    EXPECT_EQ(span.size(), 3u);
}

TEST_F(RingBufferTest, WritableContiguousOnWrappedStateMatchesSpaceUntilHead)
{
    ring_buffer<int> rb{5};
    int out = 0;

    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    rb.push_back(4);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 1);

    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 2);

    ASSERT_TRUE(rb.push_back(5));

    auto span = rb.writable_contiguous();
    EXPECT_EQ(span.size(), 1u);
}

TEST_F(RingBufferTest, CommitChunkWriteAppendsValuesInOrder)
{
    ring_buffer<int> rb{6};
    int data[] = {7, 8, 9};

    std::span<int> chunk{data};
    ASSERT_TRUE(rb.commit_chunk_write(chunk, chunk.size()));

    int out = 0;
    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 7);
    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 8);
    ASSERT_TRUE(rb.pop_front(out));
    EXPECT_EQ(out, 9);
}

TEST_F(RingBufferTest, CommitChunkWriteFailsWhenRequestedWriteExceedsCapacity)
{
    ring_buffer<int> rb{3};
    int data[] = {1, 2, 3, 4};

    std::span<int> chunk{data};
    EXPECT_FALSE(rb.commit_chunk_write(chunk, 4));
}

TEST_F(RingBufferTest, CommitChunkWriteOfZeroElementsSucceedsAndChangesNothing)
{
    ring_buffer<int> rb{3};
    int data[] = {1, 2, 3};

    std::span<int> chunk{data};
    EXPECT_TRUE(rb.commit_chunk_write(chunk, 0));
    EXPECT_TRUE(rb.empty());
}
