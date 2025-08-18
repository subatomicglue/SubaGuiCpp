find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
if(APPLE)
    find_package(moltenvk REQUIRED)
    find_library(COCOA_FRAMEWORK Cocoa)
endif()

set(SOURCE_FILES
    renderer.cpp
)
add_library(core STATIC
    ${SOURCE_FILES}
)
foreach(file ${SOURCE_FILES})
    set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "-g")
endforeach()

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PRIVATE Vulkan::Headers Vulkan::Loader )

target_link_libraries(core PUBLIC platform)
target_include_directories(core PRIVATE ../platform)

if(APPLE)
    target_link_libraries(core PUBLIC moltenvk::moltenvk ${COCOA_FRAMEWORK})
endif()
