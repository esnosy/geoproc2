#include <iostream>

#include "../libs/io/obj/obj_io.hpp"
#include "../libs/io/stl/stl_io.hpp"
#include "../libs/mat3x3.hpp"
#include "../libs/mesh_join.hpp"
#include "../libs/vec3.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/mesh.obj" << std::endl;
    return 1;
  }
  const char *filepath = argv[1];
  auto meshes = read_obj(filepath);
  auto mesh = join_meshes(meshes);
  Mat3x3 mat = {{{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}};
  for (Vec3 &v : mesh.vertices) {
    v = mat * v;
  }
  auto tris = mesh.triangulate().to_tris();
  write_stl_binary(tris, "obj_to_stl.stl");
}