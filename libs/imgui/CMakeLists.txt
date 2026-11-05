add_library(imgui STATIC
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	imgui.cpp
	imgui_impl_sdl3.cpp
	imgui_impl_sdlgpu3.cpp
)
target_include_directories(imgui PUBLIC .)
target_link_libraries(imgui PUBLIC SDL3-static)