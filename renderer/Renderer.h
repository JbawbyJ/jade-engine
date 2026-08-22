#pragma once

// Clear + draw orchestration. Phase 1 keeps this thin: no camera, no
// scene list, no material stack. Call beginFrame() then draw() each
// variable-rate render tick.

#include "math/MathTypes.h"

namespace jade {

class Mesh;
class Shader;
class Texture;

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void setClearColor(const Vec4& color);

    // Cache the camera's combined view-projection for this frame; draw()
    // uploads it as `uViewProj`. Defaults to identity so Phase 1-style
    // clip-space shaders keep working unchanged.
    void setViewProjection(const Mat4& viewProjection);

    void beginFrame() const;
    void draw(const Mesh& mesh, const Shader& shader) const;
    void draw(const Mesh& mesh, const Shader& shader, const Texture& texture) const;
    // Model-matrix overload for scene entities: uploads `uModel` alongside
    // `uViewProj`. The overloads without a model forward identity.
    void draw(const Mesh& mesh, const Shader& shader, const Texture& texture,
              const Mat4& model) const;

private:
    Vec4 m_clearColor{0.10f, 0.10f, 0.12f, 1.0f};
    Mat4 m_viewProjection{1.0f};
};

} // namespace jade
