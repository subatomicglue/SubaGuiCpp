cmake_minimum_required(VERSION 3.21)
project(SubaGuiCpp LANGUAGES CXX)

# Include Conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

# DO NOT call find_package for vulkan-loader, vulkan-headers, or moltenvk
# Conan's CMakeDeps provides targets: CONAN_PKG::vulkan-loader, etc.

# cmake -DUSE_VULKAN=ON -DUSE_OPENGL=OFF ..
option(USE_VULKAN "Build with Vulkan backend" OFF)
option(USE_OPENGL "Build with OpenGL backend" ON)
if (USE_VULKAN)
    add_compile_definitions(USE_VULKAN)
endif()
if (USE_OPENGL)
    add_compile_definitions(USE_OPENGL)
endif()

add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/gui)
add_subdirectory(examples/standalone_app)

