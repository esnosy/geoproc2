#include <chrono>
#include <iostream>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/io/off/off_io.hpp"
#include "../libs/io/ply/ply_io.hpp"
#include "../libs/io/stl/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/mesh.stl" << std::endl;
    return 1;
  }
  const char *filepath = argv[1];
  auto t0 = std::chrono::high_resolution_clock::now();
  auto tris = read_stl(filepath);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dur = t1 - t0;
  auto dur_ms_count =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  std::cout << "Read " << tris.size() << " tris in " << dur_ms_count << "ms"
            << std::endl;

  auto indexed_tri_mesh = Indexed_Tri_Mesh::from_stl_tris(tris);
  auto indexed_mesh = Indexed_Mesh::from_indexed_tri_mesh(indexed_tri_mesh);

  write_off(indexed_mesh, "ascii_to_off.off");
  write_ply_binary(indexed_mesh, "stl_to_ply.ply");

  t0 = std::chrono::high_resolution_clock::now();
  write_stl_ascii(tris, "output_ascii.stl");
  t1 = std::chrono::high_resolution_clock::now();
  dur = t1 - t0;
  dur_ms_count =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  std::cout << "Wrote " << tris.size() << " tris in " << dur_ms_count << "ms"
            << std::endl;
  return 0;
}