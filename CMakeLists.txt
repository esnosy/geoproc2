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

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
FetchContent_Declare(
	sdl
	GIT_REPOSITORY
	https://github.com/libsdl-org/SDL.git
	GIT_TAG
	release-3.4.18
)
FetchContent_MakeAvailable(sdl)

FetchContent_Declare(
	glm
	GIT_REPOSITORY
	https://github.com/g-truc/glm.git
	GIT_TAG
	1.0.3
)
FetchContent_MakeAvailable(glm)

add_subdirectory(libs)

add_executable(viewer apps/viewer.cpp)
target_link_libraries(viewer PUBLIC SDL3-static stl_io glm imgui)

if(APPLE)
	add_custom_command(
		TARGET viewer
		POST_BUILD
		COMMAND shadercross ${CMAKE_CURRENT_SOURCE_DIR}/apps/shader.vert.hlsl -o
			$<TARGET_FILE_DIR:viewer>/shader.vert.msl
		COMMAND shadercross ${CMAKE_CURRENT_SOURCE_DIR}/apps/shader.frag.hlsl -o
			$<TARGET_FILE_DIR:viewer>/shader.frag.msl
	)
else()
	add_custom_command(
		TARGET viewer
		POST_BUILD
		COMMAND shadercross ${CMAKE_CURRENT_SOURCE_DIR}/apps/shader.vert.hlsl -o
			$<TARGET_FILE_DIR:viewer>/shader.vert.spv
		COMMAND shadercross ${CMAKE_CURRENT_SOURCE_DIR}/apps/shader.frag.hlsl -o
			$<TARGET_FILE_DIR:viewer>/shader.frag.spv
	)
endif()

add_custom_command(
	TARGET viewer
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/monkey.stl $<TARGET_FILE_DIR:viewer>
)
