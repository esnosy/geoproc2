#pragma once

#include <stdexcept>

#include "vec3.hpp"

struct Triangle {
  Vec3 a, b, c;
  Vec3 &operator[](size_t i) {
    if (i == 0)
      return a;
    if (i == 1)
      return b;
    if (i == 2)
      return c;
    throw std::out_of_range("Index out of range");
  }
  const Vec3 &operator[](size_t i) const {
    if (i == 0)
      return a;
    if (i == 1)
      return b;
    if (i == 2)
      return c;
    throw std::out_of_range("Index out of range");
  }
};