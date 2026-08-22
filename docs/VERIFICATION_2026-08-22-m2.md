# Verification — 2026-08-22 (M2 First triangle)

**Repo:** `JbawbyJ/jade-engine`  
**Scope:** Phase 1 exit — MathTypes + Shader / Mesh / Texture / Renderer  
**Verdict:** Pending local build + headless run (filled in after `cmake --build` / xvfb).

---

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| Math types | `math/MathTypes.h` | `Vec2`/`Vec3`/`Vec4`/`IVec2`/`Mat3`/`Mat4` aliases over glm |
| Shader | `renderer/Shader.h/.cpp` | In-memory GLSL compile/link; `ShaderError` on failure; uniform setters |
| Mesh | `renderer/Mesh.h/.cpp` | VAO/VBO/EBO; `Vertex` = position + color + UV |
| Texture | `renderer/Texture.h/.cpp` | Real RGBA8 2D upload/bind (demo uses 1x1 white, sampled) |
| Renderer | `renderer/Renderer.h/.cpp` | `beginFrame` clear + `draw(mesh, shader[, texture])` |
| Demo | `main.cpp` | Embedded `#version 330 core` triangle; Timer/Input/Window unchanged |

`glm` is back in `vcpkg.json` and linked as `glm::glm`.

---

## Exit criteria

| Criterion | Status | Notes |
| --- | --- | --- |
| Window + logger + timer (fixed step) + input | Pass (source) | Loop shape unchanged; fixed-step drain is `TODO(jade)` |
| Renderer draws at least one mesh with a shader | Pass (source) | Colored triangle, 1x1 white texture sampled |
| ESC quits; clean shutdown log | Pass (source) | `input.isKeyDown(Key::Escape)`; existing shutdown lines |
| Zero-warning build | Pending | `-Wall -Wextra -Wpedantic` |
| Ubuntu+Windows smoke CI | Pending | No new apt packages; glm is header-only |
| Every GL call uses `GL_CHECK` | Pass (source) | Shader / Mesh / Texture / Renderer |

## Texture honesty

Texture is not a stub: constructor uploads RGBA8 via `glTexImage2D`, destructor
deletes the handle, `bind` selects a unit. The demo triangle samples a 1x1
white so vertex colors stay visible. No disk decoder (Phase 4).

## Local verify

- Build and xvfb results will be recorded after the Cloud VM compile.
