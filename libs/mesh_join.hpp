#pragma once

#include <vector>

#include "indexed_mesh.hpp"

Indexed_Mesh join_meshes(const std::vector<Indexed_Mesh> &meshes) {
  Indexed_Mesh result;
  size_t vertex_offset = 0;
  for (const auto &m : meshes) {
    for (const auto &v : m.vertices) {
      result.vertices.push_back(v);
    }
    for (const auto &f : m.faces) {
      std::vector<size_t> new_face;
      new_face.reserve(f.size());
      for (size_t vi : f) {
        new_face.push_back(vi + vertex_offset);
      }
      result.faces.push_back(new_face);
    }
    vertex_offset += m.vertices.size();
  }
  return result;
}