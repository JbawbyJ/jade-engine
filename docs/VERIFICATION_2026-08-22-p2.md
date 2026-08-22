# Verification — 2026-08-22 (M4: Phase 2 — Debug + richer GL)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 2 exit — KHR_debug callback, Camera, Renderer view-projection, Logger file sink
**Verdict:** All Phase 2 modules landed, including the stretch file sink. Locally verified
(zero-warning `-Werror` build, tests, headless run with the debug callback active).
**CI evidence pending** — push access blocked; see `VERIFICATION_2026-08-22-p20.md`.

---

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| KHR_debug callback | `core/GLDebug.h/.cpp` | `installGlDebugCallback()`: 4.3+ core or `GL_KHR_debug` extension, pointer null-checked; HIGH→ERROR, MEDIUM/LOW→WARN, notifications dropped; exception barrier; `GL_CHECK` retained as the 3.3/4.1 fallback. Compiles against both glad flavors (extension block guarded by the `GL_KHR_debug` macro — the frozen spec named the int; corrected) |
| Camera | `renderer/Camera.h/.cpp` | Pure math: yaw/pitch (radians, Y-up, −Z at rest), `view()`/`projection()`/`viewProjection()`/`forward()`, `nearPlane`/`farPlane` naming (Windows macro safety), no clamping (Phase 5 controller's job) |
| Renderer VP | `renderer/Renderer.h/.cpp` | `setViewProjection()` cached; both draw overloads upload `uViewProj`; identity default keeps clip-space shaders working |
| Demo wiring | `main.cpp` | Embedded vertex shader gains `uniform mat4 uViewProj`; static camera at (0,0,2); resize callback keeps aspect true (height-0 guarded) |
| Log file sink | `core/Logger.h/.cpp` | `setFileSink(path)`: truncating open, mirror-every-record under the existing mutex, flush on Error, deadlock-safe outcome logging (lock released before re-entering `log()`) |
| Camera tests | `tests/CameraTests.cpp` | 8 cases / 25 assertions: forward conventions, view maps position→origin and position+forward→(0,0,−1), VP composition, perspective/aspect effects |

## Exit criteria

| Criterion | Status | Evidence |
| --- | --- | --- |
| Debug callback active on 4.3+ contexts | Pass (local) | Headless llvmpipe 4.5 run logs `KHR_debug active: push-based GL diagnostics` |
| Triangle through a real view-projection | Pass (local) | Demo renders via `uViewProj`; no `Uniform … not found` warnings in the run log |
| Zero warnings | Pass (local) | `-Wall -Wextra -Wpedantic -Werror`, g++ 13.3, all targets |
| Camera unit tests | Pass | 18 doctest cases total (Timer 7, Math 3, Camera 8), all green via ctest |
| Fallback path intact | Pass (source) | On 4.1/3.3 rungs `installGlDebugCallback()` returns false → `KHR_debug unavailable: GL_CHECK polling only` |

## Deviations from the frozen spec

- `GLAD_GL_KHR_debug` in the frozen text is an `int` variable in glad 1, not a macro; the
  compile-time guard is the `GL_KHR_debug` macro. `.cursorrules` corrected to match.

## Local verify (this container, 2026-08-22)

Same loop as p20: zero-warning `-Werror` build, 18/18 tests, headless exit 0 with clean
logs. Three modules were built by parallel agents against frozen interfaces and merged
sequentially. An adversarial three-lens review (correctness / standards / GL semantics)
runs on the merged diff at this checkpoint; the standards lens is complete (its findings
are fixed in this commit), and any correctness/GL findings land as follow-up fix commits
before this branch first pushes.
