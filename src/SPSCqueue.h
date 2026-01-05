#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>

constexpr size_t cache_size = 64;

template <typename T, typename Alloc = std::allocator<T>>
class SPSCqueue : private Alloc {
public:
    using value_type = T;
    using traits = std::allocator_traits<Alloc>;
    explicit SPSCqueue(size_t capacity, Alloc allocator = Alloc{})
        : allocator_{allocator},
          capacity_{capacity},
          buffer_{traits::allocate(allocator_, capacity)} {
        if (capacity == 0)
            throw std::logic_error("Capacity of the queue has to be non-zero");
    };

    ~SPSCqueue() {
        while (!empty())
            pop_discard();
        traits::deallocate(allocator_, buffer_, capacity_);
    };

    SPSCqueue& operator=(const SPSCqueue&) = delete;
    SPSCqueue(const SPSCqueue&) = delete;

    size_t size() const {
        auto popPtr = popPtr_.load(std::memory_order_relaxed);
        auto pushPtr = pushPtr_.load(std::memory_order_relaxed);

        assert(popPtr <= pushPtr);
        return pushPtr - popPtr;
    }
    bool empty() const { return size() == 0; };
    bool full() const { return size() == capacity_; };
    T& front() {
        auto popPtr = popPtr_.load(std::memory_order_relaxed);
        return buffer_[popPtr % capacity_ ];
    }
    size_t capacity() const { return capacity_; };

    bool push(const T& value) {
        auto pushPtr = pushPtr_.load(std::memory_order_acquire);
        if (full_(pushPtr, popPtrCache_)) {
            popPtrCache_ = popPtr_.load(std::memory_order_acquire);
            if (full_(pushPtr, popPtrCache_))
                return false;
        }

        new (&buffer_[pushPtr % capacity_]) T(value);
        pushPtr_.fetch_add(1, std::memory_order_release);

        return true;
    };

    bool pop(T& out) {
        auto popPtr = popPtr_.load(std::memory_order_acquire);
        if (empty_(pushPtrCache_, popPtr)) {
            pushPtrCache_ = pushPtr_.load(std::memory_order_acquire);
            if (empty_(pushPtrCache_, popPtr))
                return false;
        }

        out = buffer_[popPtr % capacity_];
        buffer_[popPtr % capacity_].~T();
        popPtr_.fetch_add(1, std::memory_order_release);

        return true;
    };

    bool pop_discard() {
        auto popPtr = popPtr_.load(std::memory_order_acquire);
        if (empty_(pushPtrCache_, popPtr)) {
            pushPtrCache_ = pushPtr_.load(std::memory_order_acquire);
            if (empty_(pushPtrCache_, popPtr))
                return false;
        }

        buffer_[popPtr % capacity_].~T();
        popPtr_.fetch_add(1, std::memory_order_release);

        return true;
    }

private:
    Alloc allocator_;
    alignas(cache_size) std::atomic<size_t> pushPtr_{0};
    alignas(cache_size) std::atomic<size_t> popPtr_{0};
    alignas(cache_size) size_t pushPtrCache_{0};
    alignas(cache_size) size_t popPtrCache_{0};
    size_t capacity_;
    T* buffer_{nullptr};

    bool empty_(size_t pushp, size_t popp) const { return pushp == popp; };
    bool full_(size_t pushp, size_t popp) const { return pushp - popp == capacity_; };
};