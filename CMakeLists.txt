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
