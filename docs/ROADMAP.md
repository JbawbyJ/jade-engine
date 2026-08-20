# Jade Engine Roadmap

Phase tracker for the custom C++17 / OpenGL 4.6 engine. Companion docs:

- [`.cursorrules`](../.cursorrules) — full plan, coding standards, Cursor behavior
- [`VERIFICATION_2026-08-19.md`](./VERIFICATION_2026-08-19.md) — Phase 1 foundation audit

Update checkboxes when a module lands. Add a new `VERIFICATION_YYYY-MM-DD.md`
after each meaningful checkpoint.

---

## Phase 1 — Foundation (CURRENT)

| Module | Path | Status |
| --- | --- | --- |
| Build wiring (C++17, vcpkg, warnings) | `CMakeLists.txt`, `vcpkg.json` | [x] Done |
| Logger | `core/Logger.h/.cpp` | [x] Done |
| GL_CHECK helper | `core/GLDebug.h` | [x] Done |
| Window (GLFW + GL 4.6 core) | `core/Window.h/.cpp` | [x] Done |
| Entry loop + ESC | `main.cpp` | [x] Done (Input still inline) |
| Timer (delta + fixed accumulator) | `core/Timer.h/.cpp` | [x] Done |
| Input | `core/Input.h/.cpp` | [ ] **Next** |
| Math types | `math/MathTypes.h` | [ ] Open |
| Renderer | `renderer/Renderer.h/.cpp` | [ ] Open |
| Shader | `renderer/Shader.h/.cpp` | [ ] Open |
| Mesh | `renderer/Mesh.h/.cpp` | [ ] Open |
| Texture | `renderer/Texture.h/.cpp` | [ ] Open |

**Exit criteria:** fixed-timestep loop, Input module (no raw `glfwGetKey` in
`main`), renderer draws at least one mesh with a shader, clean shutdown,
zero-warning build.

**Ordered next commits:**

1. `core/Input` — replace ESC escape hatch (`TODO(jade)` already marked in `main.cpp`)
2. `math/MathTypes.h` + `renderer/` group — closes Phase 1

---

## Phase 2 — Debug + richer GL

- [ ] KHR_debug callback (replace / complement poll-style `GL_CHECK`)
- [ ] Camera / view-projection helpers
- [ ] Optional extra log sinks

## Phase 3 — Scene / transforms

- [ ] Transform hierarchy or flat entity list
- [ ] Scene ownership rules

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
| M1 Timer + Input | Timer done; Input next |
| M2 First triangle | Shader + Mesh + Renderer clear/draw |
| M3 Phase 1 exit | All Phase 1 modules checked above |
| M4+ | Open Phase 2+ sections in `.cursorrules` as they start |
