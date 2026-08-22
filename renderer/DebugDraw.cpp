#include "renderer/DebugDraw.h"

#include <glad/glad.h>

#include <cmath>
#include <cstddef>

#include "core/GLDebug.h"
#include "core/Logger.h"
#include "math/Transform.h"

namespace jade {

namespace {
    // Pass-through line shader: transform by uViewProj, hand the per-vertex
    // color straight to the fragment stage. Naming matches the asset shaders
    // (aPosition / vColor / FragColor) so shader debugging feels uniform.
    constexpr const char kLineVertexSource[] = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

uniform mat4 uViewProj;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = uViewProj * vec4(aPosition, 1.0);
}
)";

    constexpr const char kLineFragmentSource[] = R"(#version 330 core
in vec3 vColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";
} // namespace

DebugDraw::DebugDraw()
    : m_shader(kLineVertexSource, kLineFragmentSource) {
    // Same recipe as Mesh, minus the EBO — lines are drawn unindexed. The VAO
    // records the attribute layout once; flush() only rebinds and refills.
    GL_CHECK(glGenVertexArrays(1, &m_vao));
    GL_CHECK(glGenBuffers(1, &m_vbo));
    JADE_ASSERT(m_vao != 0 && m_vbo != 0, "Failed to allocate debug draw buffers");

    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    // No glBufferData here on purpose: the store is respecified every flush()
    // with whatever that frame batched (see the orphaning note there).

    const auto stride = static_cast<GLsizei>(sizeof(LinePoint));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                                   reinterpret_cast<const void*>(offsetof(LinePoint, position))));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                                   reinterpret_cast<const void*>(offsetof(LinePoint, color))));
    GL_CHECK(glEnableVertexAttribArray(1));

    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

DebugDraw::~DebugDraw() {
    if (m_vao != 0) {
        GL_CHECK(glDeleteVertexArrays(1, &m_vao));
        m_vao = 0;
    }
    if (m_vbo != 0) {
        GL_CHECK(glDeleteBuffers(1, &m_vbo));
        m_vbo = 0;
    }
}

void DebugDraw::line(const Vec3& a, const Vec3& b, const Vec3& color) {
    // Pure CPU append — GL is touched only in flush(). Batching debug lines
    // is like buffering DOM writes and committing once per frame instead of
    // issuing one draw call per segment.
    m_points.push_back({a, color});
    m_points.push_back({b, color});
}

void DebugDraw::grid(float extent, float step, const Vec3& color) {
    // A zero/negative step is a programmer error (infinite loop territory),
    // not recoverable content — hence assert, not log-and-continue.
    JADE_ASSERT(step > 0.0f, "DebugDraw::grid step must be > 0");

    // Index-based loop instead of `for (t = -extent; t <= extent; t += step)`:
    // repeated float += accumulates error and can drop the final line (the
    // classic 0.1-isn't-representable pitfall). Compute the line count once,
    // derive each coordinate by multiplication, and pin the last line to
    // +extent exactly so the border always closes.
    const int lineCount = static_cast<int>(std::lround((2.0f * extent) / step));
    for (int i = 0; i <= lineCount; ++i) {
        const float t = (i == lineCount)
            ? extent
            : -extent + step * static_cast<float>(i);
        line(Vec3{t, 0.0f, -extent}, Vec3{t, 0.0f, extent}, color); // runs along Z
        line(Vec3{-extent, 0.0f, t}, Vec3{extent, 0.0f, t}, color); // runs along X
    }
}

void DebugDraw::axes(const Transform& t, float length) {
    // w = 0 makes the matrix multiply a *direction* transform: rotation and
    // scale apply, translation does not (the same reason normals use w = 0).
    // Normalize afterwards so a scaled transform still draws axes exactly
    // `length` long — the gizmo reports orientation, not scale.
    const Mat4 m = t.matrix();
    const Vec3 xDir = glm::normalize(Vec3(m * Vec4{1.0f, 0.0f, 0.0f, 0.0f}));
    const Vec3 yDir = glm::normalize(Vec3(m * Vec4{0.0f, 1.0f, 0.0f, 0.0f}));
    const Vec3 zDir = glm::normalize(Vec3(m * Vec4{0.0f, 0.0f, 1.0f, 0.0f}));

    line(t.position, t.position + xDir * length, Vec3{1.0f, 0.0f, 0.0f}); // X = red
    line(t.position, t.position + yDir * length, Vec3{0.0f, 1.0f, 0.0f}); // Y = green
    line(t.position, t.position + zDir * length, Vec3{0.0f, 0.0f, 1.0f}); // Z = blue
}

void DebugDraw::flush(const Mat4& viewProj) {
    if (m_points.empty()) {
        return; // nothing batched this frame — skip the whole GL round trip
    }

    m_shader.bind();
    m_shader.setMat4("uViewProj", viewProj);

    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    // Respecifying the full store every flush ORPHANS the previous one: the
    // driver hands us fresh memory instead of stalling until the GPU finishes
    // reading last frame's lines — like replacing an immutable state object
    // rather than mutating one a consumer still holds.
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
                          static_cast<GLsizeiptr>(m_points.size() * sizeof(LinePoint)),
                          m_points.data(),
                          GL_DYNAMIC_DRAW));
    GL_CHECK(glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_points.size())));
    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // clear() keeps capacity, so a steady per-frame batch settles into zero
    // allocations after the first frame.
    m_points.clear();
}

} // namespace jade
