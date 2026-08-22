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
4.1 → 3.3 core** (4.1 is the macOS core-profile ceiling). Startup logs the
negotiated `GL_VERSION` and renderer. If nothing in that list is available,
window creation throws `jade::WindowError` and the process exits with a clear
error (it does not abort).

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

ESC or the window close button exits. Headless machines (CI, containers) run
the engine under a virtual display with software Mesa — llvmpipe typically
advertises 4.5, which the fallback path accepts:

```bash
LIBGL_ALWAYS_SOFTWARE=1 JADE_MAX_FRAMES=120 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/jade
```

`JADE_MAX_FRAMES=N` exits cleanly after N frames (unset or 0 = run until
close); CI's headless gate relies on it. See `AGENTS.md` for the full
headless notes, including output-buffering caveats.

## Third-party notices

| Dependency | License | Role |
| --- | --- | --- |
| [GLFW](https://www.glfw.org/) | Zlib | Window + input + context |
| [glad](https://github.com/Dav1dde/glad) | MIT | OpenGL loader (generated) |
| [glm](https://github.com/g-truc/glm) | Happy Bunny / MIT | Math types (`math/MathTypes.h`) |

Jade Engine itself is MIT. See [LICENSE](LICENSE).
