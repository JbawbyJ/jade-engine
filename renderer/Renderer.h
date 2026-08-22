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
    void beginFrame() const;
    void draw(const Mesh& mesh, const Shader& shader) const;
    void draw(const Mesh& mesh, const Shader& shader, const Texture& texture) const;

private:
    Vec4 m_clearColor{0.10f, 0.10f, 0.12f, 1.0f};
};

} // namespace jade
