#pragma once

#include "vec3.hpp"

struct Mat3x3 {
  Vec3 rows[3];

  Vec3 operator*(const Vec3 &v) const {
    return {rows[0].dot(v), rows[1].dot(v), rows[2].dot(v)};
  }

  Mat3x3 transposed() const {
    Mat3x3 result;
    result.rows[0] = {rows[0][0], rows[1][0], rows[2][0]};
    result.rows[1] = {rows[0][1], rows[1][1], rows[2][1]};
    result.rows[2] = {rows[0][2], rows[1][2], rows[2][2]};
    return result;
  }
};