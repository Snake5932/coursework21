cmake_minimum_required(VERSION 3.15)
project(shaders)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS " -static")
add_executable(shaders main.cpp glad.c perlin.h perlin.cpp loader.h loader.cpp stb_image.h stb_image.cpp)
target_link_libraries(shaders -lglu32 -lglfw3 -lopengl32 -lgdi32 -static-libgcc -static-libstdc++)