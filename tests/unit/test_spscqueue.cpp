#include <gtest/gtest.h>
#include <type_traits>
#include "SPSCqueue.h"

extern "C" {
void __ubsan_on_report() {
    FAIL() << "Encountered an undefined behavior sanitizer error";
}
void __asan_on_error() {
    FAIL() << "Encountered an address sanitizer error";
}
void __tsan_on_report() {
    FAIL() << "Encountered a thread sanitizer error";
}
}  // extern "C"

using testType = unsigned int;
class FifoTest : public testing::Test {
public:
    size_t cap = 4;
    SPSCqueue<testType> fifo{cap};
};

TEST_F(FifoTest, properties) {
    EXPECT_FALSE(std::is_default_constructible_v<SPSCqueue<testType>>);
    EXPECT_TRUE((std::is_constructible_v<SPSCqueue<testType>, unsigned long>));
    EXPECT_TRUE(
        (std::is_constructible_v<SPSCqueue<testType>, unsigned long, std::allocator<testType>>));
    EXPECT_FALSE(std::is_copy_constructible_v<SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_move_constructible_v<SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_copy_assignable_v<SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_move_assignable_v<SPSCqueue<testType>>);
    EXPECT_TRUE(std::is_destructible_v<SPSCqueue<testType>>);
}

TEST_F(FifoTest, initialState) {
    EXPECT_EQ(4u, fifo.capacity());
    EXPECT_EQ(0, fifo.size());
    EXPECT_TRUE(fifo.empty());
    EXPECT_FALSE(fifo.full());
}

TEST_F(FifoTest, push) {
    ASSERT_EQ(4u, fifo.capacity());

    EXPECT_TRUE(fifo.push(42));
    EXPECT_EQ(1u, fifo.size());
    EXPECT_FALSE(fifo.empty());
    EXPECT_FALSE(fifo.full());

    EXPECT_TRUE(fifo.push(42));
    EXPECT_EQ(2u, fifo.size());
    EXPECT_FALSE(fifo.empty());
    EXPECT_FALSE(fifo.full());

    EXPECT_TRUE(fifo.push(42));
    EXPECT_EQ(3u, fifo.size());
    EXPECT_FALSE(fifo.empty());
    EXPECT_FALSE(fifo.full());

    EXPECT_TRUE(fifo.push(42));
    EXPECT_EQ(4u, fifo.size());
    EXPECT_FALSE(fifo.empty());
    EXPECT_TRUE(fifo.full());

    EXPECT_FALSE(fifo.push(42));
    EXPECT_EQ(4u, fifo.size());
    EXPECT_FALSE(fifo.empty());
    EXPECT_TRUE(fifo.full());
}

TEST_F(FifoTest, pop) {
    auto value = testType{};
    EXPECT_FALSE(fifo.pop(value));

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        fifo.push(42 + i);
    }

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        EXPECT_EQ(fifo.capacity() - i, fifo.size());
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }
    EXPECT_EQ(0, fifo.size());
    EXPECT_TRUE(fifo.empty());
    EXPECT_FALSE(fifo.pop(value));
}

TEST_F(FifoTest, popFullFifo) {
    auto value = testType{};
    EXPECT_FALSE(fifo.pop(value));

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        ASSERT_EQ(i, fifo.size());
        ASSERT_TRUE(fifo.push(42 + i));
    }
    EXPECT_EQ(fifo.capacity(), fifo.size());
    EXPECT_TRUE(fifo.full());

    for (auto i = 0u; i < fifo.capacity() * 4; ++i) {
        EXPECT_EQ(42 + i, fifo.front());
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_FALSE(fifo.full());

        EXPECT_TRUE(fifo.push(42 + 4 + i));
        EXPECT_TRUE(fifo.full());
        EXPECT_EQ(fifo.capacity(), fifo.size());
    }
}

TEST_F(FifoTest, popEmpty) {
    auto value = testType{};
    EXPECT_FALSE(fifo.pop(value));

    for (auto i = 0u; i < fifo.capacity() * 4; ++i) {
        EXPECT_TRUE(fifo.empty());
        EXPECT_TRUE(fifo.push(42 + i));
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }

    EXPECT_TRUE(fifo.empty());
    EXPECT_FALSE(fifo.pop(value));
}

TEST_F(FifoTest, wrap) {
    auto value = testType{};
    for (auto i = 0u; i < fifo.capacity() * 2 + 1; ++i) {
        fifo.push(42 + i);
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }

    for (auto i = 0u; i < 8u; ++i) {
        fifo.push(42 + i);
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }
}

TEST_F(FifoTest, threadSafety) {
    size_t size = 1'000'000;
    SPSCqueue<testType> q{size};
    ASSERT_EQ(q.capacity(), size);

    std::thread producer([&q]() {
        for (auto i = 0; i < q.capacity(); ++i) {
            q.push(i);
        }
    });

    std::thread consumer([&q]() {
        auto removed = 0u;
        auto val = 0u;

        while (removed < q.capacity()) {
            while (q.pop(val)) {
                EXPECT_EQ(val, removed++);
            }
            usleep(1000);
        }
    });

    producer.join();
    consumer.join();
}
