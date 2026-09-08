cmake_minimum_required(VERSION 3.20)
project(geoproc2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

include(FetchContent)

FetchContent_Declare(
	fmt
	GIT_REPOSITORY
	https://github.com/fmtlib/fmt.git
	GIT_TAG
	12.2.0
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(libs)

add_executable(read_stl apps/read_stl.cpp)
target_link_libraries(read_stl PUBLIC off_io stl_io ply_io)

add_executable(read_off apps/read_off.cpp)
target_link_libraries(read_off PUBLIC off_io stl_io)

add_executable(read_ply apps/read_ply.cpp)
target_link_libraries(read_ply PUBLIC ply_io stl_io off_io)

add_executable(read_obj apps/read_obj.cpp)
target_link_libraries(read_obj PUBLIC obj_io stl_io)
