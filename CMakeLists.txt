cmake_minimum_required(VERSION 3.20)

# Pick up vcpkg toolchain BEFORE project() if VCPKG_ROOT is set.
# This lets `find_package` discover libraries vcpkg installs.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

project(JadeEngine LANGUAGES CXX)

# C++17, no compiler extensions, standard is required (not "preferred").
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put binaries in a predictable place.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# --- Dependencies (installed via vcpkg) -------------------------------------
find_package(glfw3 CONFIG REQUIRED)
find_package(glad  CONFIG REQUIRED)
find_package(glm   CONFIG REQUIRED)

# --- Engine executable ------------------------------------------------------
# As we add modules in Phase 1, append their .cpp files here.
add_executable(jade
    main.cpp
    core/Logger.cpp
    core/Window.cpp
)

# Treat the project root as an include root so we can write
# #include "core/Window.h" from anywhere.
target_include_directories(jade PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(jade PRIVATE
    glfw
    glad::glad
    glm::glm
)

# Strict warnings on day one. Future bugs hide behind warnings; we kill them now.
if(MSVC)
    target_compile_options(jade PRIVATE /W4 /permissive-)
else()
    target_compile_options(jade PRIVATE -Wall -Wextra -Wpedantic)
endif()
