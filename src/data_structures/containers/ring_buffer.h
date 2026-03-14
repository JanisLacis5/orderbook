#pragma once
#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class ring_buffer : private Alloc
{
public:
    using value_type = T;
    using traits = std::allocator_traits<Alloc>;

    explicit ring_buffer(size_t cap, Alloc allocator = Alloc{})
        : capacity_{cap}
        , allocator_{allocator}
        , buffer_{traits::allocate(allocator_, cap)}
    {
        if (cap == 0)
            throw std::logic_error("Capacity of the buffer has to be non-zero");
    }
    ~ring_buffer()
    {
        while (!empty())
            pop_back();
        traits::deallocate(allocator_, buffer_, capacity_);
    }
    ring_buffer(const ring_buffer& other) = delete;
    ring_buffer& operator=(const ring_buffer& other) = delete;
    ring_buffer(ring_buffer&& other) noexcept
        : allocator_{std::move(other.allocator_)}
        , capacity_{std::exchange(other.capacity_, 0)}
        , size_{std::exchange(other.size_, 0)}
        , buffer_{std::exchange(other.buffer_, nullptr)}
        , head_{std::exchange(other.head_, 0)}
        , tail_{std::exchange(other.tail_, 0)}
    {
    }
    ring_buffer& operator=(ring_buffer&& other) noexcept
    {
        allocator_ = std::move(other.allocator_);
        capacity_ = std::exchange(other.capacity_, 0);
        size_ = std::exchange(other.size_, 0);
        buffer_ = std::exchange(other.buffer_, nullptr);
        head_ = std::exchange(other.head_, 0);
        tail_ = std::exchange(other.tail_, 0);

        return *this;
    }

    [[nodiscard]] size_t size() const { return size_; }
    [[nodiscard]] size_t capacity() const { return capacity_; }
    [[nodiscard]] bool empty() const { return size() == 0; }
    [[nodiscard]] bool full() const { return size() == capacity_; }

    T& front() { return buffer_[head_]; }
    const T& front() const { return buffer_[head_]; }
    T& back() { return buffer_[tail_]; }
    const T& back() const { return buffer_[tail_]; }

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
        if (head_ < tail_)
            return {buffer_ + tail_, capacity() - tail_};
        return {buffer_, head_};
    }

    bool commit_chunk_write(std::span<T> chunk, size_t n)
    {
        auto data = chunk.data();
        for (auto i = 0u; i < n; ++i)
            if (!push_back(data[i]))
                return false;
        return true;
    }

private:
    Alloc allocator_;
    size_t capacity_;
    size_t size_{0};
    T* buffer_;
    size_t head_{0};
    size_t tail_{0};
};
