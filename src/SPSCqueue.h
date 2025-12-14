#pragma once

#include <cassert>
#include <cstddef>
#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class SPSCqueue : private Alloc {
public:
    explicit SPSCqueue(size_t capacity, const Alloc& allocator = Alloc{})
        : Alloc(allocator),
          capacity_{capacity},
          buffer_{std::allocator_traits<Alloc>::allocate(allocator, capacity)} {
        if (capacity == 0)
            throw std::logic_error("Capacity of the queue has to be non-zero");
    };

    ~SPSCqueue() {
        while (popPtr_ != pushPtr_) {
            buffer_[popPtr_ % capacity_].~T();
            ++popPtr_;
        }
        std::allocator_traits<Alloc>::deallocate(buffer_);
    };

    // static_assert();  assert that this is lock free

    size_t size() {
        assert(popPtr_ <= pushPtr_);
        return pushPtr_ - popPtr_;
    }
    bool empty() { return size() == 0; };
    bool full() { return size() == capacity_; };
    T* front() { return buffer_[popPtr_]; }
    bool push(const T& value);
    bool pop();
    size_t capacity() { return capacity_; };

private:
    size_t pushPtr_{0};
    size_t popPtr_{0};
    size_t capacity_;
    T* buffer_{nullptr};
};