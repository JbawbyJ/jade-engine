# Verification — 2026-08-22 (Phase 2.0: hygiene + infrastructure)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Pre-Phase-2 package: verified-findings hygiene pass + CI/test/reproducibility infrastructure
**Verdict:** All items landed and locally verified. **CI evidence pending:** pushes are
currently blocked (the Claude GitHub App lacks write access to the repo), so the three-leg
CI run, cache-save proof, and headless CI gate will be recorded when access is restored
and the branch pushes.

---

## Infrastructure → changes

| Item | Change |
| --- | --- |
| vcpkg reproducibility | `vcpkg.json` gains `builtin-baseline` = `127402f1c75bb…`; CI fetches vcpkg pinned to the same SHA instead of floating master |
| CI cache key | Now `vcpkg-<os>-<sha>-hashFiles(vcpkg.json, vcpkg-configuration.json, overlays/**)` + restore-keys — the old key omitted the overlay patches and the vcpkg commit, so the cache silently stopped saving once master moved |
| macOS leg | `macos-latest` added (configure + build + ctest; `JADE_WERROR=OFF` for its survey run, to flip on once green) |
| Headless run gate | Ubuntu CI runs `LIBGL_ALWAYS_SOFTWARE=1 JADE_MAX_FRAMES=120 xvfb-run … ./build/bin/jade`, asserts exit 0, `initialized` + `shutdown` in the log, and no GL error/`[ERROR]` lines (the expected 4.6-fallback `GLFW error 65543` excluded) |
| Frame cap | `JADE_MAX_FRAMES=N` env var: clean `requestClose()` after N frames — deterministic exit instead of `timeout` kills |
| Unit tests | doctest scaffold: `tests/TestMain.cpp`, `TimerTests.cpp` (7 cases through a new `Timer::advance()` seam), `MathTests.cpp`; `ctest` on every CI leg |
| Warnings as errors | `JADE_WERROR` option (OFF locally, ON in CI); flags shared through a `jade_warnings` interface target |
| Local dev loop | `JADE_LOCAL_GLAD_DIR` escape hatch: pre-generated glad + system glfw/glm/doctest for sandboxes that cannot run vcpkg; CI/vcpkg stay authoritative |

## Hygiene → fixes (all from the adversarially-verified review)

| Finding | Fix |
| --- | --- |
| `main` caught only `WindowError`/`ShaderError` | Catch-all `std::exception` logs and exits 1 |
| Fallback chain skipped 4.1 (macOS ceiling → silent 3.3) | `{4,6},{4,5},{4,3},{4,1},{3,3}`; docs updated everywhere the chain appears |
| Resize callback hijacked the current context | Saves and restores `glfwGetCurrentContext()` |
| C++ exceptions could unwind GLFW's C frames | try/catch barriers in the error + resize callbacks |
| Edge queries silently dead for untracked keys | Any queried key auto-enrolls in the snapshot set (first query reports false, accurate from the next frame) |
| Sub-frame taps lost between polls | `GLFW_STICKY_KEYS` / `GLFW_STICKY_MOUSE_BUTTONS` enabled |
| Shader setters mutated whichever program was bound | Setters bind their own program first |
| Missing uniforms silently swallowed; per-call location lookups | Location cache (misses included) + warn-once per bad name |
| Error log lines could scramble/lose buffered stdout | `std::cout.flush()` before every stderr error line |
| glad-before-GLFW include-order landmine | `GLFW_INCLUDE_NONE` compile definition |
| `CMAKE_SOURCE_DIR` include root | `CMAKE_CURRENT_SOURCE_DIR` (add_subdirectory-safe) |
| Stale `.cursorrules` nit; README "compile only" headless claim | Both corrected |

## Local verify (this container, 2026-08-22)

- Toolchain: g++ 13.3, CMake 3.28, Ninja; glad generated locally (`gl=4.6`, core);
  system glfw3/glm/doctest; `JADE_WERROR=ON`.
- `cmake --build`: **zero warnings** across all targets (Debug and Release).
- `ctest`: 10 test cases (Timer 7, Math 3), all pass.
- Headless: `LIBGL_ALWAYS_SOFTWARE=1 JADE_MAX_FRAMES=120 xvfb-run …` → exit 0,
  `GLXBadFBConfig` on 4.6 then **Negotiated OpenGL 4.5 core** (llvmpipe, Mesa 25.2.8),
  `Jade Engine initialized` … `Jade Engine shutdown`, no GL errors. Frame cap logged
  and honored (120 frames ≈ 0.26 s under llvmpipe, no vsync headless).

## What was not done

- No CI run yet (push access blocked) — cache-save proof and the macOS warning survey
  happen on the first successful push.
