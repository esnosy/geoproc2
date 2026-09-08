#pragma once

#include <vector>

#include "../../indexed_mesh.hpp"

std::vector<Indexed_Mesh> read_obj(const char *filepath);
void write_obj(const std::vector<Indexed_Mesh> &meshes, const char *filepath);