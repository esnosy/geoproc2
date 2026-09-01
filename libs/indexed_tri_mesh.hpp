#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include "triangle.hpp"
#include "vec3.hpp"

struct Indexed_Tri_Mesh {
  std::vector<Vec3> vertices;
  std::vector<std::array<size_t, 3>> tris;
  std::vector<Triangle> to_tris() const {
    std::vector<Triangle> result;
    result.reserve(tris.size());
    for (const auto &t : tris) {
      const Vec3 &a = vertices[t[0]];
      const Vec3 &b = vertices[t[1]];
      const Vec3 &c = vertices[t[2]];
      result.push_back({a, b, c});
    }
    return result;
  }
  static Indexed_Tri_Mesh from_stl_tris(const std::vector<Triangle> &tris) {
    Indexed_Tri_Mesh result;
    std::unordered_map<Vec3, size_t> vertex_indices;
    for (const auto &t : tris) {
      std::array<size_t, 3> indexed_tri;
      for (size_t i = 0; i < 3; i++) {
        const Vec3 &v = t[i];
        auto it = vertex_indices.find(v);
        if (it == vertex_indices.end()) {
          auto unique_vertex_index = result.vertices.size();
          vertex_indices[v] = unique_vertex_index;
          indexed_tri[i] = unique_vertex_index;
          result.vertices.push_back(v);
        } else {
          indexed_tri[i] = it->second;
        }
      }
      result.tris.push_back(indexed_tri);
    }
    return result;
  }
};