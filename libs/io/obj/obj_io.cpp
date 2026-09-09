#include <fstream>
#include <sstream>
#include <string>

#include "../../fast_float.h"
#include <fmt/os.h>

#include "../../mat3x3.hpp"
#include "obj_io.hpp"

Mat3x3 import_mat = {{{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}};
Mat3x3 export_mat = import_mat.transposed();

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
      vertex = import_mat * vertex;
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

void write_obj(const std::vector<Indexed_Mesh> &meshes, const char *filepath) {
  auto out = fmt::output_file(filepath);
  size_t vertex_offset = 1;
  size_t object_index = 0;
  for (const auto &m : meshes) {
    out.print("o mesh_{}\n", object_index);
    object_index++;
    for (Vec3 v : m.vertices) {
      v = export_mat * v;
      out.print("v {} {} {}\n", v.x, v.y, v.z);
    }
    for (const auto &f : m.faces) {
      out.print("f");
      for (size_t vi : f) {
        out.print(" {}", vi + vertex_offset);
      }
      out.print("\n");
    }
    vertex_offset += m.vertices.size();
  }
}