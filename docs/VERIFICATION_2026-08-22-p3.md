# Verification — 2026-08-22 (M5: Phase 3 — Scene / transforms)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 3 exit — Transform, flat Scene/Entity, model-matrix rendering
**Verdict:** All Phase 3 modules landed and locally verified (zero-warning `-Werror`
build, 24 unit tests, headless run drawing a four-entity scene with no GL errors).
**CI evidence pending** — push access still blocked; see `VERIFICATION_2026-08-22-p20.md`.

---

## Decision record

**Flat entity list, no hierarchy.** The demo end state never needs parenting;
`Transform::matrix()` keeps the seam open (parenting later = matrix multiplication).
**Euler over quaternions** at demo scale, order T·Ry·Rx·Rz·S, documented in
`math/Transform.h` with the swap-behind-`matrix()` upgrade note.

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| Transform | `math/Transform.h/.cpp` | Position/Euler(radians)/scale → `matrix()`; glm/gtc stays in the .cpp (rule 14) |
| Entity | `scene/Entity.h` | Name + Transform + **non-owning** `const Mesh*`/`const Texture*` (ownership rule stated in the header); renderer types forward-declared |
| Scene | `scene/Scene.h/.cpp` | `EntityId` = index; `createEntity` returns a reference documented to invalidate on the next spawn; no removal yet (`TODO(jade)`) |
| Renderer | `renderer/Renderer.h/.cpp` | `draw(mesh, shader, texture, model)` uploads `uModel`; modelless overloads forward identity so no stale per-entity state leaks between draws |
| Demo | `main.cpp` | Four transformed views of the shared triangle (center, two yawed sides, small rolled far one); shader is `uViewProj · uModel` |
| Tests | `tests/TransformTests.cpp` | 6 cases / 58 assertions incl. stepwise-derived TRS expectation and a swapped-rotation-order divergence probe |

## Exit criteria

| Criterion | Status | Evidence |
| --- | --- | --- |
| Multiple entities, distinct transforms, visible under the camera | Pass (local) | Headless 120-frame run: clean logs, `uModel`/`uViewProj` both active (no warn-once lines) |
| Ownership rules documented | Pass | `scene/Entity.h` header comment; invalidation rule in `scene/Scene.h` |
| Transform tests green | Pass | 24 total test cases / 106 assertions across the suite |
| Zero warnings | Pass (local) | `-Wall -Wextra -Wpedantic -Werror`, g++ 13.3 |

## Local verify (this container, 2026-08-22)

Same loop as p20/p2: zero-warning `-Werror` build, full ctest, headless xvfb/llvmpipe
run exits 0 with `initialized`/`shutdown` and no GL error lines. Scene and
Transform-tests modules were built by parallel agents against the frozen interfaces;
a combined three-lens review runs on the Phase 3 diff at this checkpoint, with any
findings fixed before the branch first pushes.
