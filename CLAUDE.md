# Jade Engine — CLAUDE.md

Custom C++17 game engine inspired by Ubisoft's Jade engine (branding only; original code).
**Mission: the four signature systems — world streaming, behavior/AI scripting, animation
blending, cinematic camera.** Everything else exists to serve those.

## Current state (2026-08-23)

Foundation is DONE and hardened: core (Window/Input/Logger/Timer with fixed timestep +
render interpolation), renderer (Shader/Mesh/Texture/Camera/DebugDraw), OBJ/GLSL/PNG disk
loaders, flat Scene/Entity, fly-cam + Spinner demo, 3-OS CI with headless run gate,
doctest suite (182 assertions). The demo is **frozen** — it proved the foundation works.

## Build & test (Linux/CI-style)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # vcpkg toolchain auto-picked via VCPKG_ROOT
cmake --build build                                      # strict warnings = the lint gate
ctest --test-dir build --output-on-failure
# Headless run gate (Linux only; CI runs it on every push):
LIBGL_ALWAYS_SOFTWARE=1 JADE_MAX_FRAMES=120 xvfb-run -a ./build/bin/jade
```

Sandboxes without vcpkg: `-DJADE_LOCAL_GLAD_DIR=<pre-generated glad>` + system packages
(see AGENTS.md). Never commit with a red build or failing ctest.

## Roadmap — build in THIS order (details: docs/ROADMAP.md)

1. **NOW → GLTF asset path** via tinygltf. Replaces the OBJ path (GLTF carries the
   skeletons/animations that step 5 needs; OBJ is a dead end). Exit: demo scene loads
   from .gltf, OBJ loader deleted.
2. **ECS** — EntityId + generational handles, packed ComponentPools, World with
   create/destroy/query. Components: Transform (WITH parent link — hierarchy is
   load-bearing for animation), Mesh, Camera, Behavior. Migrate flat Scene into it.
3. **Behavior layer** — FSM first, then BehaviorTree (selector/sequence/leaf) + Blackboard.
   Target: a BG&E-style NPC daily schedule as the acceptance demo.
4. **World streaming** — Zone, ZoneManager (proximity load/unload), async StreamQueue,
   WorldGraph. THE signature system. Design-first: write docs/design/streaming.md and get
   it reviewed before code.
5. **Animation blending** — ozz-animation runtime; Skeleton, AnimationClip,
   AnimationBlender (blend trees, crossfade, layer masks).
6. **CameraRig** — gameplay-aware cinematic camera ("camera as a character").

## Hard rules

- **No further packaging, release, CI, or verification-doc work.** That surface is frozen.
  Keep CI green; do not extend it.
- **No phase skipping.** Do not start item N+1 while item N is incomplete.
- One module per session. Suggest a conventional commit when it lands
  (`feat(assets): gltf mesh + material import`), and tick the box in docs/ROADMAP.md.
- For design-heavy modules (streaming, animation), start in plan mode and produce a
  design doc under docs/design/ before writing code.

## Coding standards (unchanged from Phase 1)

C++17 · `namespace jade` · `#pragma once` · PascalCase types / camelCase methods ·
RAII everywhere, no raw `new`/`delete` · no `using namespace std;` · no raw arrays ·
`JADE_ASSERT` for programmer invariants, exceptions for content errors ·
every GL call inside `GL_CHECK()` · new .cpp registered in CMakeLists immediately ·
platform code behind `#ifdef` · document the WHY at every non-obvious decision site ·
stubs tagged `// TODO(jade):` · strict warnings are the lint gate (JADE_WERROR=ON in CI).

## Files that matter

- `docs/ROADMAP.md` — canonical sequence + status. Update it when a module completes.
- `AGENTS.md` — Cursor Cloud environment notes (stale in places; trust CLAUDE.md on conflict).
- `.cursorrules` — historical Cursor-era spec; superseded by this file.
- `docs/VERIFICATION_2026-08-22-final.md` — last full audit of the frozen demo.
