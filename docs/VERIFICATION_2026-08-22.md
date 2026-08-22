# Verification — 2026-08-22

**Repo:** `JbawbyJ/jade-engine`  
**Scope:** Phase 1 review-findings hygiene (window bootstrap, glad, docs, smoke CI)  
**Verdict:** High/Medium items from the Phase 1 review are addressed. No renderer/scene/asset work.

---

## Findings → changes

| Finding | Change |
| --- | --- |
| OpenGL 4.6 hard-require + `JADE_ASSERT` abort | `Window` prefers 4.6 core, then 4.5 → 4.3 → 3.3. Total failure throws `WindowError`; `main` logs and returns 1 |
| Bare `glad` (compat / unspecified API) | `vcpkg.json` requests `glad[gl-api-46]`; overlay port sets `GLAD_PROFILE=core` (upstream has no core feature) |
| Unchecked `glGetString` | Null → `"unknown"` |
| Window size mixed with framebuffer pixels | Separate getters; `glViewport` uses FB size; FB callback makes the context current |
| GLFW refcount races / bump-before-init | Mutex; increment only after successful `glfwInit`; Rule of Five deletes documented |
| Unlicensed public repo / no README / no CI | MIT `LICENSE`, `README.md`, Ubuntu+Windows compile workflow |
| Unused `glm` / `version-string` | `glm` dropped from the link and manifest (returns with MathTypes); `"version": "0.1.0"` |

---

## What was not done

- No `math/MathTypes.h` or `renderer/` modules (intentionally still next).
- CI is compile-only; no GPU window is required on the runner.
