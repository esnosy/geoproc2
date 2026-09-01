#pragma once

#include "../../indexed_mesh.hpp"

Indexed_Mesh read_ply(const char *filepath);
void write_ply_binary(const Indexed_Mesh &mesh, const char *filepath);
void write_ply_ascii(const Indexed_Mesh &mesh, const char *filepath);