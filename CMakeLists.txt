cmake_minimum_required(VERSION 3.16)
project(udpong C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files (unity build)
set(CLIENT_SOURCES client.c)

# Option to use submodules or find installed packages
option(USE_SUBMODULES "Build SDL3 from deps/ submodules" ON)

if(USE_SUBMODULES AND EXISTS "${CMAKE_SOURCE_DIR}/deps/SDL3")
    message(STATUS "Building SDL3 libraries from submodules")

    # Add SDL3 submodules
    add_subdirectory(deps/SDL3 EXCLUDE_FROM_ALL)
    add_subdirectory(deps/SDL3_image EXCLUDE_FROM_ALL)
    add_subdirectory(deps/SDL3_ttf EXCLUDE_FROM_ALL)
    add_subdirectory(deps/SDL3_mixer EXCLUDE_FROM_ALL)
    add_subdirectory(deps/SDL3_net EXCLUDE_FROM_ALL)

    # Client executable
    add_executable(client ${CLIENT_SOURCES})

    target_link_libraries(client PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3_mixer::SDL3_mixer
        SDL3_net::SDL3_net
    )

    target_include_directories(client PRIVATE
        ${CMAKE_SOURCE_DIR}/deps/SDL3/include
        ${CMAKE_SOURCE_DIR}/deps/SDL3_image/include
        ${CMAKE_SOURCE_DIR}/deps/SDL3_ttf/include
        ${CMAKE_SOURCE_DIR}/deps/SDL3_mixer/include
        ${CMAKE_SOURCE_DIR}/deps/SDL3_net/include
    )
else()
    message(STATUS "Finding installed SDL3 packages")

    # Find SDL3 packages
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)
    find_package(SDL3_ttf REQUIRED CONFIG)
    find_package(SDL3_mixer REQUIRED CONFIG)
    find_package(SDL3_net REQUIRED CONFIG)

    # Client executable
    add_executable(client ${CLIENT_SOURCES})

    target_link_libraries(client PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3_mixer::SDL3_mixer
        SDL3_net::SDL3_net
    )
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(client PRIVATE _CRT_SECURE_NO_WARNINGS)

    # Copy DLLs to output directory on Windows
    add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:SDL3_image::SDL3_image>
            $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
            $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
            $<TARGET_FILE:SDL3_net::SDL3_net>
            $<TARGET_FILE_DIR:client>
        COMMENT "Copying SDL3 DLLs to output directory"
    )
elseif(APPLE)
    target_compile_options(client PRIVATE -Wall -Wextra)
    set_target_properties(client PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../lib;@executable_path"
    )
elseif(UNIX)
    target_compile_options(client PRIVATE -Wall -Wextra)
    target_link_libraries(client PRIVATE m)
    set_target_properties(client PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
    )
endif()

# Copy assets to build directory
add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:client>/assets
    COMMENT "Copying assets to output directory"
)

# Install rules
install(TARGETS client RUNTIME DESTINATION bin)
install(DIRECTORY assets/ DESTINATION bin/assets)

# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
