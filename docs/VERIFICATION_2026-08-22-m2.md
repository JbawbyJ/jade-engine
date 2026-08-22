# Verification — 2026-08-22 (M2 First triangle)

**Repo:** `JbawbyJ/jade-engine`  
**Scope:** Phase 1 exit — MathTypes + Shader / Mesh / Texture / Renderer  
**Verdict:** On plan. Phase 1 exit criteria met. Zero-warning build; headless GL path executes without shader or `GL_CHECK` errors.

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
| Window + logger + timer (fixed step) + input | Pass | Loop shape unchanged; fixed-step drain is `TODO(jade)` |
| Renderer draws at least one mesh with a shader | Pass | Colored triangle; 1x1 white texture sampled. Shader/mesh/texture construct before the init log |
| ESC quits; clean shutdown log | Pass (source) | `input.isKeyDown(Key::Escape)`; existing shutdown lines. Headless `timeout` cannot press ESC |
| Zero-warning build | Pass | `cmake --build build` with `-Wall -Wextra -Wpedantic`, g++ 13.3, zero warnings |
| Ubuntu+Windows smoke CI | Pass | [Actions run 32595324175](https://github.com/JbawbyJ/jade-engine/actions/runs/32595324175) — `smoke (ubuntu)` and `smoke (windows)` both green. glm header-only; no new apt packages |
| Every GL call uses `GL_CHECK` | Pass | Shader / Mesh / Texture / Renderer |

## Texture honesty

Texture is not a stub: constructor uploads RGBA8 via `glTexImage2D`, destructor
deletes the handle, `bind` selects a unit. The demo triangle samples a 1x1
white so vertex colors stay visible. No disk decoder (Phase 4).

## Local verify (Cloud VM, 2026-08-22)

```
cmake --build build
# [1/10] ... [10/10] Linking CXX executable bin/jade
# zero warnings
```

Headless (no `MESA_GL_VERSION_OVERRIDE`):

```
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/jade
```

Observed (then `timeout` SIGTERM, exit 124):

- GLFW `GLXBadFBConfig` on 4.6, then **Negotiated OpenGL 4.5 core**
- `OpenGL 4.5 (Core Profile) Mesa 25.2.8 ... | GPU: llvmpipe`
- `Jade Engine initialized` — this line is after `Shader` / `Mesh` / `Texture` construction, so compile, link, buffer upload, and `glTexImage2D` succeeded
- No `ShaderError`, no `GL_CHECK` `ERROR` lines after context creation

Draw pixels are not asserted under llvmpipe (no GPU screenshot gate). The GL path executed without error.

## CI

Commit `cc23250` on `cursor/m2-first-triangle-50ca`: both smoke jobs succeeded
(Ubuntu Ninja Release + Windows MSVC Release). That is the compile gate for
this slice; the runner does not open a window.
