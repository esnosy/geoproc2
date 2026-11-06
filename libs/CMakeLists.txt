add_subdirectory(io)
add_subdirectory(imgui)

add_library(indexed_tri_mesh STATIC indexed_tri_mesh.cpp)
target_link_libraries(indexed_tri_mesh PUBLIC glm)

add_library(indexed_mesh STATIC indexed_mesh.cpp)
target_link_libraries(indexed_mesh PUBLIC glm)

add_library(mesh_join STATIC mesh_join.cpp)
target_link_libraries(mesh_join PUBLIC glm)