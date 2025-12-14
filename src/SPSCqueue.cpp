#include "SPSCqueue.h"

template <typename T, typename Alloc>
bool SPSCqueue<T, Alloc>::push(const T& value) {
    if (full())
        return false;

    new (&buffer_[pushPtr_ % capacity_]) T(value);
    ++pushPtr_;
    return true;
}

template <typename T, typename Alloc>
bool SPSCqueue<T, Alloc>::pop() {
    if (empty())
        return false;

    buffer_[popPtr_ % capacity_].~T();
    ++popPtr_;
    return true;
}
