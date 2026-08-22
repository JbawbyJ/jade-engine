# Jade Engine Roadmap

Phase tracker for the custom C++17 / OpenGL engine (4.6 core preferred,
3.3 core minimum). Companion docs:

- [`.cursorrules`](../.cursorrules) — full plan, coding standards, Cursor behavior
- [`VERIFICATION_2026-08-19.md`](./VERIFICATION_2026-08-19.md) — Phase 1 foundation audit
- [`VERIFICATION_2026-08-22.md`](./VERIFICATION_2026-08-22.md) — Phase 1 review-findings hygiene
- [`VERIFICATION_2026-08-22-m2.md`](./VERIFICATION_2026-08-22-m2.md) — M2 first triangle / Phase 1 exit

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
| M1 Timer + Input | **complete** |
| M2 First triangle | Shader + Mesh + Renderer clear/draw — **complete** |
| M3 Phase 1 exit | All Phase 1 modules checked above — **complete** |
| M4+ | Open Phase 2+ sections in `.cursorrules` as they start |
