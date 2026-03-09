#pragma once
/// @file ring_buffer.hpp
/// Fixed-capacity ring buffer. Overwrites oldest entries when full.
/// Single-producer only (not thread-safe across producers).

#include <cstddef>
#include <cstring>
#include <vector>
#include <algorithm>

namespace wcet {

template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity) noexcept
        : data_(capacity), capacity_(capacity), head_(0), count_(0) {}

    /// Push an element. Overwrites oldest if full.
    void push(const T& item) noexcept {
        data_[head_ % capacity_] = item;
        ++head_;
        if (count_ < capacity_) ++count_;
    }

    /// Number of valid elements.
    [[nodiscard]] std::size_t size() const noexcept { return count_; }

    /// Capacity.
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

    /// Is the buffer empty?
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }

    /// Access element by index (0 = oldest valid).
    [[nodiscard]] const T& operator[](std::size_t idx) const noexcept {
        std::size_t start = (count_ < capacity_) ? 0 : (head_ % capacity_);
        return data_[(start + idx) % capacity_];
    }

    /// Get raw data pointer and count for serialization.
    [[nodiscard]] const T* data() const noexcept { return data_.data(); }

    /// Copy all valid samples into a contiguous output vector (oldest first).
    void drain(std::vector<T>& out) const {
        out.clear();
        out.reserve(count_);
        for (std::size_t i = 0; i < count_; ++i) {
            out.push_back((*this)[i]);
        }
    }

    /// Clear all entries.
    void clear() noexcept {
        head_ = 0;
        count_ = 0;
    }

private:
    std::vector<T> data_;
    std::size_t capacity_;
    std::size_t head_;
    std::size_t count_;
};

} // namespace wcet
