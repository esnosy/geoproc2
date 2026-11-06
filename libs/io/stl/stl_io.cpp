#include <fstream>
#include <iostream>
#include <print>

#include <cctype>
#include <cstdio>
#include <cstring>

#include "../../fast_float.h"
#include <fmt/os.h>

#include "../../file_buf.hpp"
#include "stl_io.hpp"

constexpr int STL_BINARY_HEADER_SIZE = 80;
constexpr int STL_BINARY_TRIANGLE_SIZE = 50;

std::vector<std::array<glm::vec3, 3>> read_stl(const char *filepath) {
  std::ifstream ifs(filepath, std::ios_base::binary);
  ifs.seekg(STL_BINARY_HEADER_SIZE, std::ios_base::cur);
  uint32_t num_tris = 0;
  ifs.read(reinterpret_cast<char *>(&num_tris), sizeof uint32_t);
  ifs.seekg(0, std::ios_base::end);
  uint64_t file_size = ifs.tellg();
  uint64_t expected_binary_file_size =
      STL_BINARY_HEADER_SIZE + sizeof uint32_t +
      STL_BINARY_TRIANGLE_SIZE * uint64_t(num_tris);

  std::vector<std::array<glm::vec3, 3>> tris;

  if (file_size == expected_binary_file_size) {
    tris.reserve(num_tris);
    ifs.seekg(STL_BINARY_HEADER_SIZE + sizeof uint32_t, std::ios_base::beg);
    for (uint32_t i = 0; i < num_tris; i++) {
      glm::vec3 normal;
      std::array<glm::vec3, 3> t;
      uint16_t attribute_byte_count;
      ifs.read(reinterpret_cast<char *>(&normal), sizeof glm::vec3);
      ifs.read(reinterpret_cast<char *>(&t), sizeof std::array<glm::vec3, 3>);
      ifs.read(reinterpret_cast<char *>(&attribute_byte_count),
               sizeof uint16_t);
      tris.push_back(t);
    }
  } else {
    ifs.seekg(0, std::ios_base::beg);
    char *buf = reinterpret_cast<char *>(malloc(file_size));
    ifs.read(buf, file_size);
    File_Buf file_buf(buf, file_size);
    while (file_buf.offset < file_buf.size) {
      file_buf.skip_spaces();
      if (file_buf.compare_token("facet")) {
        file_buf.skip_token(); // skip "facet"

        glm::vec3 normal;
        std::array<glm::vec3, 3> t;
        file_buf.skip_spaces();
        file_buf.skip_token(); // expecting "normal"

        for (int i = 0; i < 3; i++) {
          file_buf.skip_spaces();
          file_buf.parse_token_as_number(normal[i]);
        }

        // expecting "outer loop"
        file_buf.skip_spaces();
        file_buf.skip_token();
        file_buf.skip_spaces();
        file_buf.skip_token();

        for (int i = 0; i < 3; i++) {
          file_buf.skip_spaces();
          file_buf.skip_token(); // expecting "vertex"
          for (int j = 0; j < 3; j++) {
            file_buf.skip_spaces();
            file_buf.parse_token_as_number(t[i][j]);
          }
        }
        // expecting "endloop"
        file_buf.skip_spaces();
        file_buf.skip_token();
        // expecting "endfacet"
        file_buf.skip_spaces();
        file_buf.skip_token();

        tris.push_back(t);
      } else {
        file_buf.skip_token();
      }
    }
    free(buf);
  }
  return tris;
}

void write_stl_binary(const std::vector<std::array<glm::vec3, 3>> &tris,
                      const char *filepath) {
  char header[STL_BINARY_HEADER_SIZE];
  memset(header, 0, STL_BINARY_HEADER_SIZE);
  uint32_t num_tris = (uint32_t)tris.size();
  std::ofstream ofs(filepath, std::ios_base::binary | std::ios_base::trunc);
  ofs.write(header, STL_BINARY_HEADER_SIZE);
  ofs.write(reinterpret_cast<char *>(&num_tris), sizeof uint32_t);
  for (const std::array<glm::vec3, 3> &t : tris) {
    glm::vec3 normal = {0, 0, 0};
    ofs.write(reinterpret_cast<char *>(&normal), sizeof glm::vec3);
    ofs.write(reinterpret_cast<const char *>(&t),
              sizeof std::array<glm::vec3, 3>);
    uint16_t attribute_byte_count = 0;
    ofs.write(reinterpret_cast<char *>(&attribute_byte_count), sizeof uint16_t);
  }
}

void write_stl_ascii(const std::vector<std::array<glm::vec3, 3>> &tris,
                     const char *filepath) {
  auto out = fmt::output_file(filepath);
  out.print("solid \n");
  for (const std::array<glm::vec3, 3> &t : tris) {
    out.print("facet normal 0 0 0\n"
              "    outer loop\n"
              "        vertex {} {} {}\n"
              "        vertex {} {} {}\n"
              "        vertex {} {} {}\n"
              "    endloop\n"
              "endfacet\n",
              t[0].x, t[0].y, t[0].z, t[1].x, t[1].y, t[1].z, t[2].x, t[2].y,
              t[2].z);
  }
  out.print("endsolid ");
}
