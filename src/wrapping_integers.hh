#pragma once

#include <cstdint>
#include <mutex>

/*
 * The Wrap32 type represents a 32-bit unsigned integer that:
 *    - starts at an arbitrary "zero point" (initial value), and
 *    - wraps back to zero when it reaches 2^32 - 1.
 */

class Wrap32 {
  protected:
    uint32_t raw_value_{};

  public:
    explicit Wrap32(uint32_t raw_value) : raw_value_(raw_value) {}

    /* Construct a Wrap32 given an absolute sequence number n and the zero
     * point. */
    static Wrap32 wrap(uint64_t n, Wrap32 zero_point);
    Wrap32() = default;
    ~Wrap32() = default;
    Wrap32(const Wrap32 &any) { this->raw_value_ = any.raw_value_; };
    Wrap32 &operator=(const Wrap32 &other) {
        this->raw_value_ = other.raw_value_;
        return *this;
    }
    uint64_t unwrap(Wrap32 zero_point, uint64_t checkpoint) const;

    Wrap32 operator+(uint32_t n) const { return Wrap32{raw_value_ + n}; }
    bool operator==(const Wrap32 &other) const {
        return raw_value_ == other.raw_value_;
    }
};
