#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

std::vector<std::array<glm::vec3, 3>> read_stl(const char *filepath);
void write_stl_binary(const std::vector<std::array<glm::vec3, 3>> &tris, const char *filepath);
void write_stl_ascii(const std::vector<std::array<glm::vec3, 3>> &tris, const char *filepath);