cmake_minimum_required(VERSION 3.20)

# Overlay the glad port so vcpkg generates a core-profile loader.
# Must be visible before the vcpkg toolchain runs at project().
if(NOT DEFINED VCPKG_OVERLAY_PORTS)
    set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_LIST_DIR}/overlays")
endif()

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
# JADE_LOCAL_GLAD_DIR is a dev-container escape hatch: sandboxes that cannot
# run vcpkg point it at a pre-generated glad tree
# (python3 -m glad --generator=c --api="gl=4.6" --profile=core) and resolve
# glfw3/glm/doctest from system packages instead. CI + vcpkg stay authoritative.
if(JADE_LOCAL_GLAD_DIR)
    enable_language(C) # glad.c — the project is otherwise CXX-only
    add_library(jade_local_glad STATIC "${JADE_LOCAL_GLAD_DIR}/src/glad.c")
    target_include_directories(jade_local_glad SYSTEM PUBLIC "${JADE_LOCAL_GLAD_DIR}/include")
    target_link_libraries(jade_local_glad PUBLIC ${CMAKE_DL_LIBS})
    add_library(glad::glad ALIAS jade_local_glad)
else()
    find_package(glad CONFIG REQUIRED)
endif()
find_package(glfw3 CONFIG REQUIRED)
find_package(glm   CONFIG REQUIRED)

# Phase 4 asset dependencies. tinyobjloader exports the same CONFIG target
# from vcpkg and from system packages. stb is header-only with two layouts:
# vcpkg ships a find module (Stb_INCLUDE_DIR, headers unprefixed), Ubuntu
# installs under /usr/include/stb — find_path with the suffix covers both.
find_package(tinyobjloader CONFIG REQUIRED)
find_package(Stb QUIET)
if(NOT Stb_INCLUDE_DIR)
    find_path(Stb_INCLUDE_DIR stb_image.h PATH_SUFFIXES stb)
endif()
if(NOT Stb_INCLUDE_DIR)
    message(FATAL_ERROR "stb_image.h not found (vcpkg port 'stb' or system libstb-dev)")
endif()

# --- Engine executable ------------------------------------------------------
# As we add modules in Phase 1, append their .cpp files here.
add_executable(jade
    main.cpp
    assets/AssetPaths.cpp
    assets/MeshLoader.cpp
    assets/ShaderLoader.cpp
    assets/TextureLoader.cpp
    core/GLDebug.cpp
    core/Input.cpp
    core/Logger.cpp
    core/Timer.cpp
    core/Window.cpp
    game/CameraController.cpp
    game/Spinner.cpp
    math/Transform.cpp
    renderer/Camera.cpp
    renderer/Mesh.cpp
    renderer/Renderer.cpp
    renderer/Shader.cpp
    renderer/Texture.cpp
    scene/Scene.cpp
)

# Treat the project root as an include root so we can write
# #include "core/Window.h" from anywhere. CMAKE_CURRENT_SOURCE_DIR (not
# CMAKE_SOURCE_DIR) so the project still works under add_subdirectory.
target_include_directories(jade PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# GLFW must never drag system GL headers in behind glad's back; with
# GLFW_INCLUDE_NONE the glad-before-GLFW include-order trap cannot fire.
target_compile_definitions(jade PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(jade SYSTEM PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(jade PRIVATE
    glfw
    glad::glad
    glm::glm
    tinyobjloader::tinyobjloader
)

# Runtime assets live next to the binary so exe-relative resolution works the
# same from a dev tree, CI, and an unpacked release.
add_custom_command(TARGET jade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:jade>/assets"
    COMMENT "Copying assets next to the jade binary")

# Strict warnings on day one. Future bugs hide behind warnings; we kill them now.
# Shared via an interface target so every engine target (jade, jade_tests)
# compiles under the identical flag set.
# JADE_WERROR stays OFF for local dev (a newer local compiler's novel warning
# must not brick the loop); CI configures with -DJADE_WERROR=ON as the gate
# (the macOS leg joins at ON after its first green warning-survey run).
option(JADE_WERROR "Treat compiler warnings as errors" OFF)

add_library(jade_warnings INTERFACE)
if(MSVC)
    target_compile_options(jade_warnings INTERFACE /W4 /permissive-)
    if(JADE_WERROR)
        target_compile_options(jade_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(jade_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(JADE_WERROR)
        target_compile_options(jade_warnings INTERFACE -Werror)
    endif()
endif()

target_link_libraries(jade PRIVATE jade_warnings)

# --- Unit tests -------------------------------------------------------------
# GL-free logic only (Timer, math): tests never create a window or a context,
# so they run on every CI leg including macOS.
option(JADE_BUILD_TESTS "Build jade unit tests" ON)
if(JADE_BUILD_TESTS)
    find_package(doctest CONFIG REQUIRED)

    add_executable(jade_tests
        tests/TestMain.cpp
        tests/TimerTests.cpp
        tests/MathTests.cpp
        tests/CameraTests.cpp
        tests/TransformTests.cpp
        core/Timer.cpp
        math/Transform.cpp
        renderer/Camera.cpp
    )
    target_include_directories(jade_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(jade_tests PRIVATE doctest::doctest glm::glm jade_warnings)

    enable_testing()
    add_test(NAME jade_tests COMMAND jade_tests)
endif()
