#pragma once

// Immediate-mode debug line batcher (Phase 6 tooling). Call sites append
// lines each frame — line() / grid() / axes() — and flush() draws the whole
// batch in one GL_LINES call, then empties it. Think of it like an HTML5
// canvas overlay you clear and redraw every frame on top of the "real" DOM:
// nothing here persists between frames, which is exactly what makes it cheap
// to sprinkle debug visuals anywhere.
//
// The shader is a minimal embedded position+color program. Embedded is
// CORRECT here (unlike the Phase 4 asset shaders): this is engine-internal
// tooling, not content, and it must work even when the asset folder is
// missing or broken — the very situations you debug with it.
//
// REQUIRES a current OpenGL context, like every renderer type: the
// constructor creates the VAO/VBO and compiles the shader on whichever
// context is current, and flush() must run on that same context.

#include <vector>

#include "math/MathTypes.h"
#include "renderer/Shader.h"

namespace jade {

struct Transform; // math/Transform.h — only referenced here, so forward-declare (rule 14)

class DebugDraw {
public:
    /// Creates the VAO/VBO and compiles the embedded line shader.
    /// Throws ShaderError if the (embedded, so programmer-owned) GLSL fails.
    DebugDraw();
    ~DebugDraw();

    // Owns GL handles — RAII, no copies, no moves (same rule as Mesh/Shader).
    DebugDraw(const DebugDraw&) = delete;
    DebugDraw& operator=(const DebugDraw&) = delete;
    DebugDraw(DebugDraw&&) = delete;
    DebugDraw& operator=(DebugDraw&&) = delete;

    /// Append one world-space line segment from `a` to `b` in `color`.
    void line(const Vec3& a, const Vec3& b, const Vec3& color);

    /// XZ-plane grid at y = 0: lines every `step` from -extent to +extent
    /// along both axes. `step` must be > 0 (programmer invariant).
    void grid(float extent, float step, const Vec3& color);

    /// The transform's local X/Y/Z axes as red/green/blue segments of
    /// `length`, starting at t.position. Handy pose gizmo for any entity.
    void axes(const Transform& t, float length);

    /// Upload the batch (GL_DYNAMIC_DRAW, respecified per flush so the driver
    /// can orphan the old storage), draw GL_LINES, and clear the batch
    /// (capacity kept). No-op when the batch is empty.
    void flush(const Mat4& viewProj);

private:
    // Vertex layout is private to the batcher: location 0 = position,
    // location 1 = color, interleaved.
    struct LinePoint {
        Vec3 position;
        Vec3 color;
    };

    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    Shader m_shader;                 // embedded line shader, built in the ctor
    std::vector<LinePoint> m_points; // CPU-side batch; cleared, never shrunk
};

} // namespace jade
