# Jade Engine — Roadmap (re-anchored 2026-08-23)

Mission: rebuild the design philosophy of Ubisoft's Jade engine — **world streaming,
behavior/AI scripting, animation blending, cinematic camera** — as original C++17 systems.
BG&E is the design document: every system answers *"would this have shipped BG&E?"*

> The 2026-08-22 demo sprint (fly-cam scene, packaging, 3-OS CI) is **complete and
> frozen**. It closed the original Month 1–2 foundation milestone and half of Month 3–4.
> Its phase numbering in older docs/commit messages is superseded by this file.

---

## Milestone ladder

### M1 — Foundation ✅ COMPLETE (frozen)
Window/Input/Logger/Timer (fixed step + interpolation), renderer stack, OBJ/GLSL/PNG
loaders, flat Scene, fly camera, DebugDraw, doctest suite, 3-OS CI + headless gate,
packaging. No further investment in this surface.

### M2 — GLTF asset path 🔶 CURRENT
- [ ] tinygltf dependency (vcpkg manifest + CMake)
- [ ] Mesh import: positions/normals/UVs/indices from .gltf/.glb
- [ ] Material/texture binding from GLTF materials
- [ ] Demo scene content converted; OBJ loader and .obj assets deleted
- [ ] Loader unit tests (parse fixtures, no GL)
**Why first:** GLTF carries skeletons + animation clips — M5 depends on this format.
**Exit:** demo scene renders identically from GLTF, CI green.

### M3 — Entity-Component System
- [ ] EntityId with generational handles (stale-handle safety)
- [ ] Packed ComponentPool<T> (cache-coherent iteration)
- [ ] World: create/destroy/query
- [ ] Components: Transform (**with parent link** — hierarchy is load-bearing for M5),
      MeshRenderer, Camera, Behavior
- [ ] Flat Scene migrated; Spinner/CameraController become systems over the World
**Exit:** demo runs on the ECS with entity destroy + handle reuse under test.

### M4 — Behavior / AI scripting
- [ ] StateMachine (states, transitions, conditions) — unit-tested, GL-free
- [ ] BehaviorTree: selector / sequence / leaf + decorators
- [ ] Blackboard shared memory
- [ ] Acceptance: a BG&E-style NPC daily schedule (wander → work → react) in the demo world
- [ ] (defer Lua/sol2 until two+ behaviors exist in C++)

### M5 — World streaming  ⭐ signature system
- [ ] docs/design/streaming.md FIRST — zones, budgets, load states, failure modes
      (reference: BG&E GDC material, Ubisoft streaming patents, UE level streaming)
- [ ] Zone: serialized chunk of entities + assets
- [ ] StreamQueue: async background loading (worker thread + main-thread GL upload)
- [ ] ZoneManager: proximity load/unload with hysteresis
- [ ] WorldGraph: adjacency + transition triggers
**Exit:** walk a 3×3-zone world with no hitch > one fixed step; zones visibly
load/unload in DebugDraw.

### M6 — Animation blending
- [ ] ozz-animation runtime integration
- [ ] Skeleton + AnimationClip import from GLTF (via M2)
- [ ] AnimationBlender: blend trees, crossfade, layer masks
**Exit:** a character idles→walks→runs with smooth crossfades under the fixed step.

### M7 — Cinematic camera (CameraRig)
- [ ] Follow / orbit / cinematic modes with constraint volumes
- [ ] Gameplay-aware framing ("camera as a character")
**Exit:** rig demo through the streamed world with an animated character.

### M8 — Vertical slice
Small BG&E-style level exercising every system above. (The noir "Perisphere"
concept is a candidate setting.)

---

## Standing rules
- Build strictly in order; no M(N+1) work while M(N) is open.
- Design-heavy milestones (M5, M6) require a reviewed design doc before code.
- CI stays green; the frozen M1 surface (packaging/release/CI) is not extended.
- Tick boxes here in the same commit that completes them.
