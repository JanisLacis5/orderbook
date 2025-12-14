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
    using value_type = typename SPSCqueue<testType>::value_type;
    SPSCqueue<testType> fifo{4};
};

TEST_F(FifoTest, properties) {
    EXPECT_FALSE(std::is_default_constructible_v<typename SPSCqueue>);
    EXPECT_TRUE((std::is_constructible_v<typename SPSCqueue<testType>, unsigned long>));
    EXPECT_TRUE((std::is_constructible_v<typename SPSCqueue<testType>, unsigned long,
                                         std::allocator<typename value_type>>));
    EXPECT_FALSE(std::is_copy_constructible_v<typename SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_move_constructible_v<typename SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_copy_assignable_v<typename SPSCqueue<testType>>);
    EXPECT_FALSE(std::is_move_assignable_v<typename SPSCqueue<testType>>);
    EXPECT_TRUE(std::is_destructible_v<typename SPSCqueue<testType>>);
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
    auto value = typename value_type{};
    EXPECT_FALSE(fifo.pop(value));

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        fifo.push(42 + i);
    }

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        EXPECT_EQ(fifo.capacity() - i, fifo.size());
        auto value = typename value_type{};
        EXPECT_TRUE(fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }
    EXPECT_EQ(0, fifo.size());
    EXPECT_TRUE(fifo.empty());
    EXPECT_FALSE(fifo.pop(value));
}

TEST_F(FifoTest, popFullFifo) {
    auto value = typename value_type{};
    EXPECT_FALSE(fifo.pop());

    for (auto i = 0u; i < fifo.capacity(); ++i) {
        ASSERT_EQ(i, fifo.size());
        ASSERT_TRUE(fifo.push(42 + i));
    }
    EXPECT_EQ(fifo.capacity(), fifo.size());
    EXPECT_TRUE(fifo.full());

    for (auto i = 0u; i < fifo.capacity() * 4; ++i) {
        EXPECT_EQ(42 + i, fifo.front());
        EXPECT_TRUE(fifo.pop());
        EXPECT_FALSE(fifo.full());

        EXPECT_TRUE(fifo.push(42 + 4 + i));
        EXPECT_TRUE(fifo.full());
        EXPECT_EQ(fifo.capacity(), fifo.size());
    }
}

TEST_F(FifoTest, popEmpty) {
    auto value = typename value_type{};
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
    auto value = typename value_type{};
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
