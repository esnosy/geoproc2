#pragma once

#include <array>
#include <vector>

#include <glm/glm.hpp>

struct Indexed_Tri_Mesh {
  std::vector<glm::vec3> vertices;
  std::vector<std::array<size_t, 3>> tris;

  std::vector<std::array<glm::vec3, 3>> to_tris() const;

  static Indexed_Tri_Mesh
  from_stl_tris(const std::vector<std::array<glm::vec3, 3>> &tris);
};