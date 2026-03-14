#include <algorithm>
#include <cstddef>

template <typename T>
class ring_buffer
{
public:
    ring_buffer(size_t cap)
        : capacity_{cap}
    {
    }
    ~ring_buffer() {} // TODO: implement
    ring_buffer(ring_buffer& other)
    {
        using std::swap;
        swap(other.capacity_, capacity_);

        other.buffer_ = new T;
        other.buffer_ = buffer_;
        buffer_ = nullptr;

        return other;
    }

private:
    size_t capacity_;
    T* buffer_;
};
