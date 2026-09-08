#pragma once

#include "vec3.hpp"

struct Mat3x3 {
  Vec3 rows[3];

  Vec3 operator*(const Vec3 &v) const {
    return {rows[0].dot(v), rows[1].dot(v), rows[2].dot(v)};
  }
};