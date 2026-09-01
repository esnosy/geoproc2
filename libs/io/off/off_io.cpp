#include <fstream>
#include <string>

#include <cstdlib>

#include <fmt/os.h>

#include "../../file_buf.hpp"
#include "off_io.hpp"

Indexed_Mesh read_off(const char *filepath) {
  std::ifstream ifs(filepath, std::ios_base::binary);
  ifs.seekg(0, std::ios_base::end);
  auto file_size = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);
  char *buf = reinterpret_cast<char *>(malloc(file_size));
  File_Buf file_buf(buf, file_size);
  std::string line;
  // Strip comments from file
  while (std::getline(ifs, line)) {
    for (char c : line) {
      if (c == '#') {
        break;
      }
      file_buf.buf[file_buf.offset++] = c;
    }
    file_buf.buf[file_buf.offset++] = '\n';
  }
  file_buf.size = file_buf.offset;
  file_buf.offset = 0;

  file_buf.skip_spaces();
  file_buf.skip_token(); // expecting "OFF"
  size_t num_vertices, num_faces, num_edges;
  file_buf.skip_spaces();
  file_buf.parse_token_as_number(num_vertices);

  file_buf.skip_spaces();
  file_buf.parse_token_as_number(num_faces);

  file_buf.skip_spaces();
  file_buf.parse_token_as_number(num_edges);

  Indexed_Mesh result;
  result.vertices.reserve(num_vertices);
  result.faces.reserve(num_faces);

  for (size_t i = 0; i < num_vertices; i++) {
    Vec3 v;
    file_buf.skip_spaces();
    file_buf.parse_token_as_number(v.x);

    file_buf.skip_spaces();
    file_buf.parse_token_as_number(v.y);

    file_buf.skip_spaces();
    file_buf.parse_token_as_number(v.z);

    file_buf.skip_until('\n');
    result.vertices.push_back(v);
  }

  for (size_t i = 0; i < num_faces; i++) {
    size_t num_face_vertices;
    file_buf.skip_spaces();
    file_buf.parse_token_as_number(num_face_vertices);
    std::vector<size_t> &face = result.faces.emplace_back();
    face.reserve(num_face_vertices);
    for (size_t j = 0; j < num_face_vertices; j++) {
      size_t vertex_index;
      file_buf.skip_spaces();
      file_buf.parse_token_as_number(vertex_index);
      face.push_back(vertex_index);
    }
    file_buf.skip_until('\n');
  }

  free(buf);
  return result;
}

void write_off(const Indexed_Mesh &mesh, const char *filepath) {
  auto out = fmt::output_file(filepath);
  out.print("OFF\n");
  out.print("{} {} 0\n", mesh.vertices.size(), mesh.faces.size());
  for (auto v : mesh.vertices) {
    out.print("{} {} {}\n", v.x, v.y, v.z);
  }
  for (auto f : mesh.faces) {
    out.print("{}", f.size());
    for (auto vi : f) {
      out.print(" {}", vi);
    }
    out.print("\n");
  }
}
