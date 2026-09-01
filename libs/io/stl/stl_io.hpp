#pragma once

#include <vector>

#include "../../triangle.hpp"

std::vector<Triangle> read_stl(const char *filepath);
void write_stl_binary(const std::vector<Triangle> &tris, const char *filepath);
void write_stl_ascii(const std::vector<Triangle> &tris, const char *filepath);