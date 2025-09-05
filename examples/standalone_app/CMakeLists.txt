set(SOURCE_FILES
    main.cpp
)
add_executable(standalone_app
    ${SOURCE_FILES}
)
foreach(file ${SOURCE_FILES})
    set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "-g")
endforeach()

# Vulkan!
if (USE_VULKAN)
    # ---------------------------find library/package---------------------------
    find_package(VulkanLoader REQUIRED)
    find_package(VulkanHeaders REQUIRED)
    if(APPLE)
        find_package(moltenvk REQUIRED)
        find_library(COCOA_FRAMEWORK Cocoa)

        # Get the library directory for MoltenVK
        # Use the find_library() to find if it's available
        find_library(MOLTENVK_LIBRARY NAMES MoltenVK PATHS ${CMAKE_FIND_LIBRARY_HINTS} REQUIRED)

        # Print the directory where the MoltenVK libraries are located
        if(MOLTENVK_LIBRARY)
            get_filename_component(MOLTENVK_LIB_DIR ${MOLTENVK_LIBRARY} DIRECTORY)
            message(STATUS "MoltenVK library directory: ${MOLTENVK_LIB_DIR}")

            # Write the MOLTENVK_LIB_DIR to a shell script file
            set(SOURCEME_SH "${CMAKE_CURRENT_BINARY_DIR}/sourceme.sh")
            file(WRITE ${SOURCEME_SH} "#!/bin/bash\n")
            file(APPEND ${SOURCEME_SH} "export VK_ICD_FILENAMES=\"${MOLTENVK_LIB_DIR}/MoltenVK_icd.json\"\n")
            file(APPEND ${SOURCEME_SH} "export VK_LAYER_PATH=\"${MOLTENVK_LIB_DIR}\"\n")

            # Make the script executable (optional)
            #file(CHANGE_MODE "sourceme.sh" OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        else()
            message(WARNING "MoltenVK library not found!")
        endif()
    endif()

    # ---------------------------include + link---------------------------
    target_link_libraries(standalone_app PRIVATE Vulkan::Headers)
    target_link_libraries(standalone_app PRIVATE Vulkan::Loader)
    if(APPLE)
        target_include_directories(standalone_app PRIVATE ${CONAN_MOLTENVK_INCLUDE_DIRS})
        target_link_libraries(standalone_app PRIVATE moltenvk::moltenvk ${COCOA_FRAMEWORK})
    endif()
endif()


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")
find_package(PNG REQUIRED)
find_package(nlohmann_json REQUIRED)

# Link against core and platform
target_include_directories(standalone_app PRIVATE ../../src/platform)
target_include_directories(standalone_app PRIVATE ../../src/core)
target_include_directories(standalone_app PRIVATE ../../src/gui)
# target_link_libraries(standalone_app PRIVATE core)
# target_link_libraries(standalone_app PRIVATE platform)
target_link_libraries(standalone_app PRIVATE guikit)
# target_link_libraries(standalone_app PRIVATE nlohmann_json::nlohmann_json)
# target_link_libraries(standalone_app PRIVATE PNG::PNG)
