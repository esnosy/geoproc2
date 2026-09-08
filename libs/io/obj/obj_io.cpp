#include <fstream>
#include <sstream>
#include <string>

#include "../../fast_float.h"

#include "obj_io.hpp"

std::vector<Indexed_Mesh> read_obj(const char *filepath) {
  std::vector<Indexed_Mesh> meshes;
  size_t current_vertex_offset = 1;
  size_t current_vertex_index = 1;

  std::ifstream ifs(filepath);
  std::string line;
  std::string token;
  while (std::getline(ifs, line)) {
    std::stringstream line_ss(line);
    line_ss >> token;
    if (token == "o") {
      meshes.push_back(Indexed_Mesh());
      current_vertex_offset = current_vertex_index;
    } else if (token == "v") {
      if (meshes.empty()) {
        meshes.push_back(Indexed_Mesh());
      }
      current_vertex_index++;
      Vec3 vertex;
      line_ss >> token;
      fast_float::from_chars(token.data(), token.data() + token.size(),
                             vertex.x);
      line_ss >> token;
      fast_float::from_chars(token.data(), token.data() + token.size(),
                             vertex.y);
      line_ss >> token;
      fast_float::from_chars(token.data(), token.data() + token.size(),
                             vertex.z);
      meshes.back().vertices.push_back(vertex);
    } else if (token == "f") {
      std::vector<size_t> face;
      while (line_ss >> token) {
        size_t i;
        fast_float::from_chars(token.data(), token.data() + token.size(), i);
        face.push_back(i - current_vertex_offset);
      }
      meshes.back().faces.push_back(face);
    }
  }
  return meshes;
}