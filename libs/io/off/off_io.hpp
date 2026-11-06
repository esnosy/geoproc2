#pragma once

#include <vector>

#include <cstdint>

#include "../../indexed_mesh.hpp"

Indexed_Mesh read_off(const char *filepath);
void write_off(const Indexed_Mesh &mesh, const char *filepath);