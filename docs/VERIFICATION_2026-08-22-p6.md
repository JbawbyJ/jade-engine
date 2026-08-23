# Verification — 2026-08-22 (M8: Phase 6 — Tooling / editor hooks)

**Repo:** `JbawbyJ/jade-engine`
**Scope:** Phase 6 exit — debug draw overlay, multi-window proof
**Verdict:** Both Phase 6 items landed and locally verified. The log file sink already
landed in Phase 2 (did not slip). **CI evidence pending** — push access still blocked;
see `VERIFICATION_2026-08-22-p20.md`.

---

## Modules → files

| Module | Files | Notes |
| --- | --- | --- |
| Debug draw | `renderer/DebugDraw.h/.cpp` | Immediate-mode line batcher: one dynamic VBO respecified (orphaned) per flush, embedded minimal line shader (correct here — engine tooling must work when assets are broken), `line`/`grid`/`axes`/`flush`, every GL call in `GL_CHECK` |
| F1 toggle | `main.cpp` | Grid (extent 3, step 0.5) + per-entity sim-state axes flushed after the lit pass |
| Multi-window proof | `main.cpp`, `core/Window.h/.cpp` | `JADE_SECOND_WINDOW=1` constructs + destroys a second window before GL resources exist; new `Window::makeCurrent()` restores the primary context afterwards |

## Exit criteria

| Criterion | Status | Evidence |
| --- | --- | --- |
| Grid + axes render, F1 toggles | Pass (source + startup) | Headless runs compile/link the line shader at startup under llvmpipe (DebugDraw constructs unconditionally); toggle path uses the tracked F1 edge query. Headless cannot press F1 — visual confirmation is a desktop check |
| Second window: balanced init/terminate, clean shutdown | Pass (headless) | Run log shows two `Negotiated OpenGL … core context` lines, `Second window constructed; destroying it again`, `primary context restored`, then a normal full render run and clean shutdown — no GL errors |
| Zero warnings, tests green | Pass | `-Werror` g++ 13.3; 31 cases / 182 assertions |

## Local verify (this container, 2026-08-22)

DebugDraw built by one agent against the frozen interface with a CPU-side harness
proving the batch math (grid 26 lines with exact ±extent endpoints; rotated axes
directions; flush clear-keep-capacity). Coordinator gate: zero-warning `-Werror`
build, full ctest, headless runs with and without `JADE_SECOND_WINDOW=1`, all exit 0
with clean logs. A combined review of the Phase 6 + Phase 7 diffs runs before the
branch first pushes.
