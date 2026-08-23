# Verification — 2026-08-22 (M6: Phase 4 — Assets)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 4 exit — normals prerequisite, disk loaders, asset content, demo scene from files
**Verdict:** All Phase 4 modules landed and locally verified: the demo now loads its
shader, texture, and meshes from `assets/` next to the binary and renders a Lambert-lit
checker scene headless with zero GL errors. **CI evidence pending** — push access still
blocked; see `VERIFICATION_2026-08-22-p20.md`.

---

## Decision record

stb (images) + tinyobjloader (OBJ meshes) + plain-text GLSL files; loaders throw
`AssetError` (content errors are recoverable — never asserts); ownership transfers via
`std::unique_ptr` (rule 4 allowance, the GL types are non-movable). Hot reload skipped
per the original plan.

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| Normals prerequisite | `renderer/Mesh.h/.cpp`, shaders, `renderer/Renderer.*` | Vertex layout 0=pos, 1=normal, 2=color, 3=uv; proper normal matrix in the vertex shader; wrapped Lambert over a 0.15 ambient floor; `Renderer::setLightDirection` (normalized, zero-vector-safe) uploads `uLightDir` |
| AssetError / AssetPaths | `assets/AssetError.h`, `assets/AssetPaths.h/.cpp` | `assetRoot()`: `JADE_ASSET_ROOT` override → exe-relative `assets/` (per-platform query behind `#if`) → CWD fallback with one WARN; magic-static, logged once |
| TextureLoader | `assets/TextureLoader.h/.cpp` | stb_image (single implementation TU), vertical flip, forced RGBA8, RAII pixel guard, `AssetError` with path + stb reason |
| MeshLoader | `assets/MeshLoader.h/.cpp` | tinyobjloader (triangulate), de-indexed into the normal-bearing layout, flat normals generated when missing (degenerate-safe), empty mesh → `AssetError` not assert |
| ShaderLoader | `assets/ShaderLoader.h/.cpp` | Binary read-to-string before any GL work; delegates to the from-memory ctor (ShaderError untouched) |
| Content | `assets/shaders/basic.*`, `assets/meshes/cube.obj`, `plane.obj`, `assets/textures/checker.png` | Shaders extracted verbatim from the embedded GLSL; hand-authored OBJs (winding/normal agreement machine-verified); 181-byte checker PNG spot-checked through stb |
| Build | `CMakeLists.txt`, `vcpkg.json`, `.gitignore` | stb + tinyobjloader deps (both vcpkg and system layouts); post-build asset copy next to the binary; MSVC `*.obj` ignore rule exempted for `assets/meshes` |

## Exit criteria

| Criterion | Status | Evidence |
| --- | --- | --- |
| Demo renders disk-loaded shaders + lit textured meshes | Pass (local) | Headless run log: `Asset root: …/bin/assets`, shader/texture/OBJ load lines (cube 12 tris, plane 2), clean 120-frame run |
| Bad path exits gracefully | Pass (agent-tested) | Loader harnesses: missing files throw `AssetError` carrying the full resolved path; no aborts |
| Exe-relative resolution in a clean environment | Pass (local) | Binary run from a build dir the source tree never referenced; assets found next to the binary via the POST_BUILD copy |
| Zero warnings | Pass (local) | `-Wall -Wextra -Wpedantic -Werror`, g++ 13.3 |
| Tests still green | Pass | 24 cases / 106 assertions |

## Local verify (this container, 2026-08-22)

Loaders were built by four parallel agents against frozen interfaces (two coding
against stub headers for the not-yet-landed contract files, revalidated against the
real ones), each with per-module functional harnesses (env-override/exe-relative
resolution, missing-file throws, OBJ de-indexing against authored fixtures, PNG
round-trip through stb). Coordinator merge gate: zero-warning `-Werror` build, full
ctest, headless xvfb/llvmpipe run exit 0 with clean logs. A combined three-lens review
runs on the Phase 4 diff at this checkpoint; findings land as fix commits before the
branch first pushes.
