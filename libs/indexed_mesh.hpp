#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "indexed_tri_mesh.hpp"

struct Indexed_Mesh {
  std::vector<glm::vec3> vertices;
  std::vector<std::vector<size_t>> faces;

  Indexed_Tri_Mesh triangulate() const;

  static Indexed_Mesh
  from_indexed_tri_mesh(const Indexed_Tri_Mesh &indexed_tri_mesh);
};