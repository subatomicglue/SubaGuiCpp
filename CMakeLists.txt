cmake_minimum_required(VERSION 3.21)
project(MyVulkanApp LANGUAGES CXX)

# Include Conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

# DO NOT call find_package for vulkan-loader, vulkan-headers, or moltenvk
# Conan's CMakeDeps provides targets: CONAN_PKG::vulkan-loader, etc.

add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(examples/standalone_app)

