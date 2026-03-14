#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <span>

// TODO: allow custom allocator
template <typename T>
class ring_buffer
{
public:
    explicit ring_buffer(size_t cap)
        : capacity_{cap}
    {
        assert(cap != 0);
        // TODO: alloc memory
    }
    ~ring_buffer() {} // TODO: implement
    ring_buffer(const ring_buffer& other)
    {
        using std::swap;
        swap(other.capacity_, capacity_);

        other.buffer_ = new T;
        other.buffer_ = buffer_;
        buffer_ = nullptr;
    }
    ring_buffer& operator=(const ring_buffer& other)
    {
        // TODO: implement
        return *this;
    }
    ring_buffer(ring_buffer&& other) noexcept
    {
        // TODO implement
    }
    ring_buffer& operator=(ring_buffer&& other) noexcept
    {
        // TODO: implement
    }

    [[nodiscard]] size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size() == 0; }
    [[nodiscard]] bool full() const { return size() == capacity_; }

    T& front() { return buffer_[head_]; }
    const T& front() const { return buffer_[head_]; }
    T& back() { return buffer_[tail_]; }
    const T& back() const { return buffer_[tail_]; }

    std::span<const T> readable_contiguous() const
    {
        if (head_ < tail_) {
            auto blockSize = tail_ - head_;
            return {buffer_[head_], blockSize};
        }
        return {buffer_, tail_};
    }

    std::span<T> writable_contiguous()
    {
        if (head_ < tail_) {
            auto blockSize = tail_ - head_;
            return {buffer_[head_], blockSize};
        }
        return {buffer_, tail_};
    }

    // TODO: implement
    bool consume_front(size_t n) {}
    bool consume_back(size_t n) {}

    bool push_back(const T& val) {}
    bool push_back(T&& val) {}
    bool pop_back(T& out) {}
    std::optional<T> pop_back() {}

    bool push_front(const T& val) {}
    bool push_front(T&& val) {}
    bool pop_front(T& out) {}
    std::optional<T> pop_front() {}
    // TODO end

private:
    size_t capacity_;
    size_t size_{0};

    T* buffer_;

    size_t head_{0};
    size_t tail_{0};
};
