#pragma once

#include <vector>

#include "indexed_tri_mesh.hpp"
#include "vec3.hpp"

struct Indexed_Mesh {
  std::vector<Vec3> vertices;
  std::vector<std::vector<size_t>> faces;

  Indexed_Tri_Mesh triangulate() const {
    Indexed_Tri_Mesh result;
    result.vertices = vertices;
    for (const auto &f : faces) {
      for (size_t i = 2; i < f.size(); i++) {
        result.tris.push_back({f[0], f[i - 1], f[i]});
      }
    }
    return result;
  }

  static Indexed_Mesh
  from_indexed_tri_mesh(const Indexed_Tri_Mesh &indexed_tri_mesh) {
    Indexed_Mesh result;
    result.vertices = indexed_tri_mesh.vertices;
    for (const auto &t : indexed_tri_mesh.tris) {
      std::vector<size_t> face;
      face.push_back(t[0]);
      face.push_back(t[1]);
      face.push_back(t[2]);
      result.faces.push_back(face);
    }
    return result;
  }
};