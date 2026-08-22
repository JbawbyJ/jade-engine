# Jade Engine Roadmap

Phase tracker for the custom C++17 / OpenGL engine (4.6 core preferred,
3.3 core minimum). Companion docs:

- [`.cursorrules`](../.cursorrules) — full plan, coding standards, Cursor behavior
- [`VERIFICATION_2026-08-19.md`](./VERIFICATION_2026-08-19.md) — Phase 1 foundation audit
- [`VERIFICATION_2026-08-22.md`](./VERIFICATION_2026-08-22.md) — Phase 1 review-findings hygiene
- [`VERIFICATION_2026-08-22-m2.md`](./VERIFICATION_2026-08-22-m2.md) — M2 first triangle / Phase 1 exit
- [`VERIFICATION_2026-08-22-p20.md`](./VERIFICATION_2026-08-22-p20.md) — Phase 2.0 hygiene + infrastructure
- [`VERIFICATION_2026-08-22-p2.md`](./VERIFICATION_2026-08-22-p2.md) — M4 Phase 2 debug + camera
- [`VERIFICATION_2026-08-22-p3.md`](./VERIFICATION_2026-08-22-p3.md) — M5 Phase 3 scene + transforms

Update checkboxes when a module lands. Add a new `VERIFICATION_YYYY-MM-DD.md`
after each meaningful checkpoint.

---

## Phase 1 — Foundation (DONE)

| Module | Path | Status |
| --- | --- | --- |
| Build wiring (C++17, vcpkg, warnings) | `CMakeLists.txt`, `vcpkg.json` | [x] Done |
| Logger | `core/Logger.h/.cpp` | [x] Done |
| GL_CHECK helper | `core/GLDebug.h` | [x] Done |
| Window (GLFW + GL 4.6→3.3 fallback) | `core/Window.h/.cpp` | [x] Done |
| Entry loop + ESC | `main.cpp` | [x] Done (Renderer + Input) |
| Timer (delta + fixed accumulator) | `core/Timer.h/.cpp` | [x] Done |
| Input | `core/Input.h/.cpp` | [x] Done |
| Math types | `math/MathTypes.h` | [x] Done |
| Renderer | `renderer/Renderer.h/.cpp` | [x] Done |
| Shader | `renderer/Shader.h/.cpp` | [x] Done |
| Mesh | `renderer/Mesh.h/.cpp` | [x] Done |
| Texture | `renderer/Texture.h/.cpp` | [x] Done (1x1 RGBA upload/bind; sampled by the demo triangle) |

**Exit criteria:** fixed-timestep loop, Input module (no raw `glfwGetKey` in
`main`), renderer draws at least one mesh with a shader, clean shutdown,
zero-warning build. **Met.**

**Ordered next commits:**

1. Phase 2 when explicitly kicked off — do not invent camera / KHR_debug / asset loaders yet.

---

## Phase 2 — Debug + richer GL (DONE)

- [x] KHR_debug callback (`core/GLDebug.cpp` — complements poll-style `GL_CHECK`,
      which stays as the 3.3/4.1 fallback)
- [x] Camera / view-projection helpers (`renderer/Camera.h/.cpp`; demo renders
      through `uViewProj`)
- [x] Extra log sinks (`Logger::setFileSink` file mirror)

Phase 2.0 pre-package also landed: verified-findings hygiene pass, vcpkg
pinning, macOS CI leg, headless CI run gate, doctest unit tests, JADE_WERROR.
CI runs for these checkpoints are pending push access (see verification docs).

## Phase 3 — Scene / transforms (DONE)

- [x] Flat entity list (decision: no hierarchy; `Transform::matrix()` keeps the
      seam open) — `scene/Entity.h`, `scene/Scene.h/.cpp`, `math/Transform.h/.cpp`
- [x] Scene ownership rules (non-owning mesh/texture pointers; creator owns —
      stated in `scene/Entity.h`; `EntityId` over references across spawns)

## Phase 4 — Assets

- [ ] Mesh / texture / shader loading from disk

## Phase 5 — Gameplay loop

- [ ] Systems driven by the fixed timestep

## Phase 6 — Tooling / editor hooks

- [ ] Multi-window / debug draw

## Phase 7 — Polish + packaging

- [ ] Release config, packaging, end-user README

---

## Milestone timeline (technical, not calendar)

| Milestone | Meaning |
| --- | --- |
| M0 First Task | Window + Logger + GL_CHECK + clear loop — **complete** |
| M1 Timer + Input | **complete** |
| M2 First triangle | Shader + Mesh + Renderer clear/draw — **complete** |
| M3 Phase 1 exit | All Phase 1 modules checked above — **complete** |
| M4 Phase 2 | KHR_debug + Camera + log sink — **complete (local verify; CI pending)** |
| M5 Phase 3 | Transform + flat scene + model-matrix draw — **complete (local verify; CI pending)** |
| M6+ | Open Phase 4+ sections in `.cursorrules` as they start |
