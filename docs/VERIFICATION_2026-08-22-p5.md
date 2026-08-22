# Verification — 2026-08-22 (M7: Phase 5 — Gameplay loop)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 5 exit — fixed-step systems, snapshot/blend interpolation, fly camera
**Verdict:** All Phase 5 modules landed and locally verified (zero-warning `-Werror`
build, 31 unit-test cases, headless interpolated-loop run with clean logs). **CI
evidence pending** — push access still blocked; see `VERIFICATION_2026-08-22-p20.md`.

---

## Decision record

**Snapshot + blend interpolation** (`interpolate(previous, current, Timer::alpha())`
per entity and for the camera pose) — fixed-step motion stays smooth at any refresh
rate. **No System base class** — two concrete classes called explicitly from the
drain. **Mouse look latched to one fixed step per render frame** — `mouseDelta()` is a
per-frame quantity, so banked catch-up steps must not multiply the turn.

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| Timer alpha | `core/Timer.h` | `[0,1)` fraction of the next step already elapsed, post-drain |
| Pose interpolation | `math/Transform.h/.cpp` | Lerp position/scale; IEEE-exact shortest-arc Euler blend (`std::remainder`); composes through `Transform::matrix()` so TRS order has one definition |
| Snapshot support | `scene/Entity.h`, `scene/Scene.h/.cpp` | `previousTransform` + `Scene::snapshotPrevious()` before the drain |
| Mouse delta | `core/Input.h/.cpp` | Per-update cursor delta, constructor-seeded (no first-frame spike); `mousePosition()` now frame-coherent; `Key::F1` added |
| Fly camera | `game/CameraController.h/.cpp` | Authoritative pose + previous copies; WASD planar (diagonal-normalized), Space/LeftShift vertical, right-drag look (latched once per frame), pitch clamped ±89° here per the Phase 2 freeze; `writeToCamera(alpha)` blends |
| Spinner | `game/Spinner.h/.cpp` | Fixed-step Y rotation prover; yaw wrapped by `std::remainder` (seam invisible under shortest-arc blending); stale ids assert per the no-removal invariant |
| Loop wiring | `main.cpp` | The Phase 1 `TODO(jade)` drain hook is finally consumed: snapshot → fixed steps → interpolated render |

## Exit criteria

| Criterion | Status | Evidence |
| --- | --- | --- |
| Systems consume the fixed timestep | Pass | Controller + spinner run only inside the drain at `fixedDelta()` |
| Render on the variable frame, simulate on the fixed step | Pass | Render path touches only interpolated poses; headless 180-frame run clean |
| Fly camera (WASD + mouse), ESC clean shutdown | Pass (source + headless) | Headless can't press keys; movement math unit-anchored via Camera tests; ESC path unchanged and edge-queried |
| Interpolation correct | Pass | 7 dedicated test cases incl. ±pi wrap against stepwise-derived expectations |
| Tests green, zero warnings | Pass | 31 cases / 145+ assertions; `-Werror` g++ 13.3 |

## Local verify (this container, 2026-08-22)

Four parallel agents (interpolation math + tests, Input delta, controller, spinner)
against frozen interfaces; per-module functional harnesses (spinner 600k-step
wrap-boundedness, controller pose round-trip, delta seeding). Coordinator merge gate:
zero-warning `-Werror` build, full ctest, headless xvfb/llvmpipe run exit 0, no GL
errors. A three-lens review runs on the Phase 5 diff at this checkpoint; findings land
as fix commits before the branch first pushes.
