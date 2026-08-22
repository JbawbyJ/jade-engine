# Jade Engine

A small original C++17 / OpenGL engine that ships a playable demo scene: a fly
camera over a Lambert-lit, checker-textured ground with spinning cubes, every
shader, mesh, and texture loaded from disk. Simulation runs on a fixed
timestep; rendering interpolates between the last two simulation states.
Dependencies (`glfw3`, `glad`, `glm`, `stb`, `tinyobjloader`, `doctest` for
tests) are managed with vcpkg and wired in through CMake.

The name is inspired by Ubisoft's Jade engine **as branding only**. This is an
original project. It is not Ubisoft IP, not affiliated with Ubisoft, and not
a fork of hogsy/jaded or any other existing Jade reimplementation.

## Run the demo

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` | Move (planar — along the horizon relative to where you look) |
| `Space` / `Left Shift` | Up / down |
| Hold **Right Mouse** + drag | Look around |
| `F1` | Toggle the debug grid + axes overlay |
| `ESC` | Quit |

### Get a build

CI packages the demo per platform on every run: a `jade-<os>` artifact
(`jade-ubuntu`, `jade-windows`, `jade-macos`) containing
`jade-engine-<version>-<system>.zip` (Windows) or `.tar.gz` (Linux / macOS).
Unpack it and run `./jade` (`jade.exe` on Windows). The `assets/` folder must
stay next to the binary — assets resolve relative to the executable. Setting
the `JADE_ASSET_ROOT` environment variable overrides the asset location.

Building from source, `cmake --build build --target package` produces the same
archive locally.

## Build from source

- CMake 3.20+
- A C++17 compiler
- [vcpkg](https://github.com/microsoft/vcpkg) (`VCPKG_ROOT` exported, or pass
  `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`)
- An OpenGL **3.3 core** (or newer) driver

```bash
export VCPKG_ROOT=/path/to/vcpkg   # skip if already set

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/jade
```

When `VCPKG_ROOT` is set, a plain configure picks up the vcpkg toolchain on
its own. `vcpkg.json` pins a `builtin-baseline`, so the first configure
installs the exact dependency set CI uses. Ninja is preferred but not
required; multi-config generators (Visual Studio) write the binary to
`build/bin/<Config>/jade.exe` instead of `build/bin/jade`.

### OpenGL minimum

The engine **prefers OpenGL 4.6 core**, then falls back through **4.5 → 4.3 →
4.1 → 3.3 core** (4.1 is the macOS core-profile ceiling). Startup logs the
negotiated `GL_VERSION` and renderer. If nothing in that list is available,
window creation throws `jade::WindowError` and the process exits with a clear
error (it does not abort).

**Hard minimum: OpenGL 3.3 core.** The glad loader is generated for the 4.6
core API (`gl-api-46` plus the `overlays/glad` port setting
`GLAD_PROFILE=core`); a 3.3 context still loads.

### Tests

GL-free unit tests (doctest: timer, math, camera) build by default:

```bash
ctest --test-dir build --output-on-failure
```

### Build options

| Option | Default | Meaning |
| --- | --- | --- |
| `JADE_BUILD_TESTS` | `ON` | Build the `jade_tests` doctest runner (run via `ctest`) |
| `JADE_WERROR` | `OFF` | Treat warnings as errors. Off for local dev; CI's Linux and Windows legs configure with `-DJADE_WERROR=ON` as the gate |
| `JADE_LOCAL_GLAD_DIR` | unset | Escape hatch for sandboxes that cannot run vcpkg: point it at a pre-generated GL 4.6 core glad tree and resolve the other dependencies from system packages |

## Headless machines (CI, containers)

No GPU or display is needed — run under a virtual X server with Mesa's
software rasterizer (llvmpipe typically advertises 4.5, which the fallback
path accepts):

```bash
LIBGL_ALWAYS_SOFTWARE=1 JADE_MAX_FRAMES=120 \
  xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/jade
```

- `JADE_MAX_FRAMES=N` exits cleanly after N frames (unset or 0 = run until
  close); CI's headless gate relies on it.
- `JADE_SECOND_WINDOW=1` constructs and destroys a second window at startup —
  the multi-window / context-restore proof.

See `AGENTS.md` for the full headless notes, including output-buffering
caveats.

## Third-party notices

| Dependency | License | Role |
| --- | --- | --- |
| [GLFW](https://www.glfw.org/) | Zlib | Window + input + context |
| [glad](https://github.com/Dav1dde/glad) | MIT | OpenGL loader (generated) |
| [glm](https://github.com/g-truc/glm) | Happy Bunny / MIT | Math types (`math/MathTypes.h`) |
| [doctest](https://github.com/doctest/doctest) | MIT | Unit tests (`tests/`, build-time only) |
| [stb](https://github.com/nothings/stb) | MIT / public domain | Image decoding (`stb_image`) |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) | MIT | OBJ mesh loading |

Jade Engine itself is MIT. See [LICENSE](LICENSE).

## Development

`.cursorrules` is the durable plan: project identity, coding standards, and
the full phase-by-phase history. `docs/ROADMAP.md` tracks module status, and
the dated `docs/VERIFICATION_*.md` files record what was verified at each
checkpoint. Development is phase-gated — finish the current module before
opening the next.
