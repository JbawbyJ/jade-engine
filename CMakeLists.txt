# 3.21 for TARGET_RUNTIME_DLLS (the Windows package must carry the vcpkg
# dynamic-triplet DLLs or the unpacked exe dies at load).
cmake_minimum_required(VERSION 3.21)

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

# Bump vcpkg.json's "version" together with this — the manifest cannot read
# CMake variables, so the two declarations must be kept in lockstep by hand.
project(JadeEngine VERSION 0.1.0 LANGUAGES CXX)

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
# from vcpkg and from system packages. stb is header-only with two layouts
# (vcpkg: unprefixed under the vcpkg include root; Ubuntu: /usr/include/stb)
# and ships NO find module or config anywhere — the find_path below is the
# real resolution on every platform, not a fallback. Do not delete it.
find_package(tinyobjloader CONFIG REQUIRED)
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
    renderer/DebugDraw.cpp
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
# JADE_VERSION feeds the startup log from the one authoritative version.
target_compile_definitions(jade PRIVATE
    GLFW_INCLUDE_NONE
    JADE_VERSION="${PROJECT_VERSION}")

if(MSVC)
    # main.cpp reads env vars through standard std::getenv (read-only, at
    # startup, single-threaded); MSVC's C4996 "unsafe, use _dupenv_s" advisory
    # would be fatal under /WX, so opt out of that one advisory rather than
    # fork the code per platform.
    target_compile_definitions(jade PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(jade SYSTEM PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(jade PRIVATE
    glfw
    glad::glad
    glm::glm
    tinyobjloader::tinyobjloader
)

# Runtime assets live next to the binary so exe-relative resolution works the
# same from a dev tree, CI, and an unpacked release. A standalone ALL target
# (not POST_BUILD) so editing only a shader/texture still syncs on the next
# build — POST_BUILD would skip when the binary is up to date. Content dirs
# only: assets/ also holds loader sources, which must not ship next to the
# binary. Note copy_directory never deletes: a removed asset lingers in the
# build dir until a clean build (rename-test on a clean checkout).
add_custom_target(jade_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/meshes"   "$<TARGET_FILE_DIR:jade>/assets/meshes"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"  "$<TARGET_FILE_DIR:jade>/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures" "$<TARGET_FILE_DIR:jade>/assets/textures"
    COMMENT "Syncing content dirs next to the jade binary")
add_dependencies(jade_assets jade)

# --- Packaging (Phase 7) ----------------------------------------------------
# One flat package: the binary at the root with assets/ beside it, exactly
# the layout exe-relative resolution expects. Content dirs only — loader
# sources stay out of the package for the same reason they stay out of the
# build dir. ZIP on Windows, TGZ elsewhere; `cmake --build build --target
# package` (or cpack) produces jade-engine-<version>-<system>.<ext>.
install(TARGETS jade RUNTIME DESTINATION .)
if(WIN32)
    # vcpkg's default dynamic triplet builds glfw3/tinyobjloader as DLLs;
    # applocal copies them next to the BUILD-tree exe only, so the package
    # must install them too or the shipped exe fails to load.
    install(FILES $<TARGET_RUNTIME_DLLS:jade> DESTINATION .)
endif()
install(DIRECTORY assets/meshes assets/shaders assets/textures DESTINATION assets)
set(CPACK_PACKAGE_NAME "jade-engine")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME
    "jade-engine-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)

# Strict warnings on day one. Future bugs hide behind warnings; we kill them now.
# Shared via an interface target so every engine target (jade, jade_tests)
# compiles under the identical flag set.
# JADE_WERROR stays OFF for local dev (a newer local compiler's novel warning
# must not brick the loop); every CI leg configures with -DJADE_WERROR=ON as
# the gate (macOS joined after a clean AppleClang warning-survey run).
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
