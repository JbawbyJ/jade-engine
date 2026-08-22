# Jade Engine

Custom C++17 game engine (Phase 1). It opens a GLFW window, creates an OpenGL
core-profile context (prefers 4.6, falls back to 4.5 / 4.3 / 3.3), and draws a
colored triangle every frame until you press ESC or close the window.
Dependencies (`glfw3`, `glad`, `glm`) are managed with vcpkg and wired in through
CMake. `glad` is generated for GL 4.6 **core** via `gl-api-46` plus the
`overlays/glad` port (`GLAD_PROFILE=core`).

## Cursor Cloud specific instructions

Setup dependencies (system libs, vcpkg at `$HOME/vcpkg`, prebuilt dependency binary
cache) are already installed in the environment and persisted in the VM snapshot. The
startup update script pre-configures the CMake `build/` directory. The notes below are
the non-obvious things to know when developing here.

### Build

- vcpkg lives at `$HOME/vcpkg` and `VCPKG_ROOT` is exported in `~/.bashrc`. `CMakeLists.txt`
  reads `VCPKG_ROOT` to auto-select the vcpkg toolchain, so a plain configure works. If you
  configure from a non-login shell where `VCPKG_ROOT` is unset, pass the toolchain explicitly:
  `-DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"`.
- Configure (also triggers vcpkg to install manifest deps into `build/vcpkg_installed/`):
  `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"`
- Build: `cmake --build build`. The executable is written to `build/bin/jade`.
- Ninja is the generator. The first configure on a cold cache builds glfw3/glad from
  source; afterwards vcpkg's binary cache makes it fast.

### Lint

- There is no separate linter. The build enforces strict warnings (`-Wall -Wextra -Wpedantic`),
  so treat a clean `cmake --build build` as the lint gate.

### Test

- There is no automated test suite in this repo yet. Verify changes by building and running.

### Run (headless caveat — important)

The VM has no GPU or physical display, so the engine must run under a virtual X server
(`Xvfb`) with Mesa software rendering. llvmpipe typically advertises OpenGL 4.5; the
engine falls back from 4.6, so a version override is optional. Run it like this:

```bash
LIBGL_ALWAYS_SOFTWARE=1 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/jade
```

To force the advertised version (useful if you want to exercise the 4.6 path):

```bash
LIBGL_ALWAYS_SOFTWARE=1 MESA_GL_VERSION_OVERRIDE=4.6 MESA_GLSL_VERSION_OVERRIDE=460 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/jade
```

- The render loop runs until ESC/window-close; headless there is no input, so wrap it in
  `timeout <seconds>` when you just want to confirm startup.
- Startup logs (`OpenGL ... | GPU: ...` and `Jade Engine initialized`) go to `std::cout`,
  which is fully buffered when redirected to a file. If you redirect to a log file and kill
  the process, prefix with `stdbuf -oL -eL` or the buffer will never flush and the log looks
  empty.
