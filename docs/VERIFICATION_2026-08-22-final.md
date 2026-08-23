# Verification — 2026-08-22 (M-final: Phase 7 + roadmap completion)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 7 exit and the full-roadmap acceptance (M-final)
**Verdict:** Phases 2.0 through 7 are complete and verified end-to-end, including
the packaged-binary acceptance flow — locally and now in CI.

**CI evidence (added 2026-08-23, once push access was granted):**

- [Run 32646675625](https://github.com/JbawbyJ/jade-engine/actions/runs/32646675625)
  — first cloud run, cold caches: **Ubuntu green** (build, ctest, headless gate,
  package smoke, `jade-ubuntu` artifact, cache saved), **macOS green** (clean
  AppleClang 21 warning survey, `jade-engine-0.1.0-Darwin-arm64.tar.gz` uploaded,
  cache saved), **Windows red** on exactly one issue: MSVC C4996 (`std::getenv`
  advisory) fatal under `/WX` — the WERROR-surprise risk the plan predicted.
- Fixes: `_CRT_SECURE_NO_WARNINGS` on MSVC for the two startup env reads, and the
  macOS survey exemption ended in the same push (`JADE_WERROR=ON` on all legs).
- [Run 32646951393](https://github.com/JbawbyJ/jade-engine/actions/runs/32646951393)
  — **all three legs green in ~2.5 minutes** with warm caches and warnings-as-errors
  everywhere: the cache key fix, pinning, and per-leg package artifacts all proven.

---

## M-final acceptance (what "complete" means, demonstrated)

Unpack the package (locally: `jade-engine-0.1.0-Linux-x86_64.tar.gz` from
`cpack`; in CI: the `jade-<os>` artifact) and run `jade`:

| Criterion | Status | Evidence |
| --- | --- | --- |
| Package is self-contained (binary + content `assets/`) | Pass (local) | Tarball inspected: `jade` at root, `assets/{meshes,shaders,textures}` only — no loader sources; Windows DLL install rule present (`TARGET_RUNTIME_DLLS`), CI-run pending |
| Packaged binary runs from a fresh directory | Pass (local) | Asset root resolves exe-relative inside the unpack dir; version 0.1.0 logged; clean 60/120-frame headless runs, zero GL errors |
| Fly camera over a lit, disk-loaded scene | Pass (local + source) | Checker plane + three cubes (two spinning) through the interpolated fixed-step loop; WASD/Space/LeftShift/right-drag per README |
| F1 debug overlay, ESC clean shutdown | Pass (source + startup) | Line shader compiles under llvmpipe at startup; toggles are tracked edge queries; shutdown log asserted by every gate run |
| Multi-window proof | Pass (local) | `JADE_SECOND_WINDOW=1` in the packaged run: two contexts negotiated, balanced teardown, primary restored |
| Zero-warning `-Werror` builds | Pass (local g++ 13.3) | Debug + Release; macOS/MSVC legs pending CI |
| Unit tests | Pass | 31 doctest cases / 182 assertions via ctest |
| Reproducible deps | Pass (structural) | vcpkg pinned by `builtin-baseline` + pinned CI fetch; cache key covers overlays and the SHA |

## Phase 7 items

`project VERSION 0.1.0` → `JADE_VERSION` startup log; CPack flat package
(ZIP/Windows, TGZ elsewhere, arch in the filename); CI packages every leg and
uploads `jade-<os>` artifacts; package smoke step (Ubuntu unpacks and runs the
tarball) and `JADE_SECOND_WINDOW=1` added to the headless gate; end-user README
(controls first, then build-from-source, full third-party notices).
**Performance note (honest, per the original rule): not profiled; no known
hotspots at demo scale.**

## Final-review findings → resolutions

| Finding | Resolution |
| --- | --- |
| Windows ZIP shipped without vcpkg DLLs (would fail at load on user machines) | `install(FILES $<TARGET_RUNTIME_DLLS:jade>)` on WIN32; CMake minimum raised to 3.21; CI package smoke added so this class of break can never ship green again |
| Bare `cpack` relied on implicit config on the VS leg | `cpack -C Release` |
| Package name lacked the architecture | `-<system>-<processor>` suffix |
| "One authoritative version" was actually two | Lockstep comment at `project()`; `vcpkg.json` must be bumped together |
| Performance note missing | Landed in README (above) |

## Annotations (informational, no code change)

- `VERIFICATION_2026-08-22-p4.md` describes the asset sync as a POST_BUILD copy;
  it has since become the always-run `jade_assets` target (dated docs stay as
  written; this note is the correction).
- The second-window proof leaves glad's process-global pointers loaded from the
  destroyed second context — safe for this proof on GLX/EGL/CGL (and WGL with an
  identical pixel format), but `Window::makeCurrent()` does not re-load glad, so
  long-lived multi-window switching on per-context-pointer drivers would need a
  reload step. Recorded for the future editor work.
- Two commits in the range are unbuildable snapshots (`assets/` copy step landed
  before the assets; TextureLoader/Spinner landed before their CMake wiring, per
  the amended rule 9's disclosed-deferral allowance). `git bisect` across this
  branch should use `--first-parent` sensibilities and skip those two.

## Process record

The run was executed by a coordinating session fanning out implementation
agents per phase against interfaces frozen in `.cursorrules`, with per-module
functional harnesses, a coordinator merge gate (zero-warning `-Werror` build +
ctest + headless xvfb/llvmpipe run with log assertions) before every commit,
and adversarial multi-lens reviews per phase whose findings all landed as fix
commits. The review chain caught, among others: an interpolation scheme that
degenerated to a stair-step at high refresh rates (spec bug), framerate-scaled
mouse sensitivity, an OBJ-index heap over-read, and the Windows packaging gap.
