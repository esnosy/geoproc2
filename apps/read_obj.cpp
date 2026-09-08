#include <iostream>

#include "../libs/io/obj/obj_io.hpp"
#include "../libs/io/stl/stl_io.hpp"
#include "../libs/mesh_join.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/mesh.obj" << std::endl;
    return 1;
  }
  const char *filepath = argv[1];
  auto meshes = read_obj(filepath);
  auto mesh = join_meshes(meshes);
  auto tris = mesh.triangulate().to_tris();
  write_stl_binary(tris, "obj_to_stl.stl");
}