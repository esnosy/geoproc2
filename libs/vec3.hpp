#pragma once

#include <stdexcept>

struct Vec3 {
  float x, y, z;

  float &operator[](size_t i) {
    if (i == 0)
      return x;
    if (i == 1)
      return y;
    if (i == 2)
      return z;
    throw std::out_of_range("Index out of range");
  }

  float operator[](size_t i) const {
    if (i == 0)
      return x;
    if (i == 1)
      return y;
    if (i == 2)
      return z;
    throw std::out_of_range("Index out of range");
  }

  float dot(const Vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  bool operator==(const Vec3 &other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

// form Boost
inline size_t hash_mix(size_t x) {
  x ^= x >> 32;
  x *= 0xe9846af9b1a615d;
  x ^= x >> 32;
  x *= 0xe9846af9b1a615d;
  x ^= x >> 28;
  return x;
}

// from Boost
template <class T> inline void hash_combine(std::size_t &seed, T const &v) {
  seed = hash_mix(seed + 0x9e3779b9 + std::hash<T>()(v));
}

namespace std {
template <> struct hash<Vec3> {
  size_t operator()(const Vec3 &v) const {
    size_t seed = 0;
    hash_combine(seed, v.x);
    hash_combine(seed, v.y);
    hash_combine(seed, v.z);
    return seed;
  }
};
} // namespace std