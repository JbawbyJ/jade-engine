# Jade Engine

A small C++17 OpenGL engine scaffold. Phase 1 opens a GLFW window, creates a
core-profile context, and draws a colored triangle each frame until you press
ESC or close the window.

The name is inspired by Ubisoft's Jade engine **as branding only**. This is an
original project. It is not Ubisoft IP, not affiliated with Ubisoft, and not
a fork of hogsy/jaded or any other existing Jade reimplementation.

## Requirements

- CMake 3.20+
- A C++17 compiler
- [vcpkg](https://github.com/microsoft/vcpkg) (`VCPKG_ROOT` exported, or pass
  `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`)
- An OpenGL **3.3 core** (or newer) driver

### OpenGL minimum

The engine **prefers OpenGL 4.6 core**, then falls back through **4.5 → 4.3 →
3.3 core**. Startup logs the negotiated `GL_VERSION` and renderer. If nothing
in that list is available, window creation throws `jade::WindowError` and the
process exits with a clear error (it does not abort).

**Hard minimum: OpenGL 3.3 core.**

The GLAD loader is generated for the 4.6 core API (`gl-api-46` plus a vcpkg
overlay that sets `GLAD_PROFILE=core`). A 3.3 context still loads; 4.x entry
points stay unused until a later phase needs them.

## Build

```bash
export VCPKG_ROOT=/path/to/vcpkg   # skip if already set

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

The binary is written to `build/bin/jade` (or `build/bin/Debug/jade` with the
Visual Studio generator).

Ninja is preferred but not required. The first configure builds `glfw3`,
`glad`, and `glm` via the vcpkg manifest.

## Run

```bash
./build/bin/jade
```

ESC or the window close button exits. Headless / CI machines with no GPU
should compile only; a live window needs a display and a GL 3.3+ driver.
Software Mesa (llvmpipe) typically advertises 4.5 and is accepted by the
fallback path.

## Third-party notices

| Dependency | License | Role |
| --- | --- | --- |
| [GLFW](https://www.glfw.org/) | Zlib | Window + input + context |
| [glad](https://github.com/Dav1dde/glad) | MIT | OpenGL loader (generated) |
| [glm](https://github.com/g-truc/glm) | Happy Bunny / MIT | Math types (`math/MathTypes.h`) |

Jade Engine itself is MIT. See [LICENSE](LICENSE).
