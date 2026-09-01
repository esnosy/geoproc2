#include <chrono>
#include <iostream>

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
  auto mesh = read_ply(filepath);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dur = t1 - t0;
  auto dur_ms_count =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  std::cout << "Read " << mesh.faces.size() << " faces in " << dur_ms_count
            << "ms" << std::endl;
  auto tris = mesh.triangulate().to_tris();
  write_stl_binary(tris, "ply_to_stl.stl");
}