#pragma once

#include <cassert>
#include <cstddef>
#include <memory>

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
        assert(popPtr_ <= pushPtr_);
        return pushPtr_ - popPtr_;
    }
    bool empty() const { return size() == 0; };
    bool full() const { return size() == capacity_; };
    T& front() { return buffer_[popPtr_ % capacity_ ]; }
    size_t capacity() const { return capacity_; };
    bool push(const T& value) {
        if (full())
            return false;

        new (&buffer_[pushPtr_ % capacity_]) T(value);
        ++pushPtr_;
        return true;
    };
    bool pop(T& out) {
        if (empty())
            return false;

        out = buffer_[popPtr_ % capacity_];
        buffer_[popPtr_ % capacity_].~T();
        ++popPtr_;

        return true;
    };
    bool pop_discard() {
        if (empty())
            return false;

        buffer_[popPtr_ % capacity_].~T();
        ++popPtr_;

        return true;
    }

private:
    Alloc allocator_;
    std::atomic<size_t> pushPtr_{0};
    std::atomic<size_t> popPtr_{0};
    size_t capacity_;
    T* buffer_{nullptr};
};