#include <unordered_map>

#include "indexed_tri_mesh.hpp"

std::vector<std::array<glm::vec3, 3>> Indexed_Tri_Mesh::to_tris() const {
  std::vector<std::array<glm::vec3, 3>> result;
  result.reserve(tris.size());
  for (const auto &t : tris) {
    const glm::vec3 &a = vertices[t[0]];
    const glm::vec3 &b = vertices[t[1]];
    const glm::vec3 &c = vertices[t[2]];
    result.push_back({a, b, c});
  }
  return result;
}

// from Boost
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

struct GLM_Vec3_Hasher {
  size_t operator()(const glm::vec3 &v) const {
    size_t seed = 0;
    hash_combine(seed, v.x);
    hash_combine(seed, v.y);
    hash_combine(seed, v.z);
    return seed;
  }
};

Indexed_Tri_Mesh Indexed_Tri_Mesh::from_stl_tris(
    const std::vector<std::array<glm::vec3, 3>> &tris) {
  Indexed_Tri_Mesh result;
  std::unordered_map<glm::vec3, size_t, GLM_Vec3_Hasher> vertex_indices;
  for (const auto &t : tris) {
    std::array<size_t, 3> indexed_tri;
    for (size_t i = 0; i < 3; i++) {
      const glm::vec3 &v = t[i];
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