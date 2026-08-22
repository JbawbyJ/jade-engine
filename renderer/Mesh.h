#pragma once

// VAO / VBO / EBO wrapper for a static indexed mesh.
// Vertex layout: position + normal + color + UV (locations 0 / 1 / 2 / 3).
// Normals landed with Phase 4 so disk-loaded meshes can be Lambert-lit.

#include <cstddef>
#include <cstdint>

#include "math/MathTypes.h"

namespace jade {

struct Vertex {
    Vec3 position{0.0f, 0.0f, 0.0f};
    Vec3 normal{0.0f, 0.0f, 1.0f}; // default faces +Z, like a screen-aligned quad
    Vec3 color{1.0f, 1.0f, 1.0f};
    Vec2 texCoord{0.0f, 0.0f};
};

class Mesh {
public:
    Mesh(const Vertex* vertices, std::size_t vertexCount,
         const std::uint32_t* indices, std::size_t indexCount);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    void bind() const;
    void unbind() const;
    void draw() const;

    std::size_t indexCount() const { return m_indexCount; }

private:
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_ebo{0};
    std::size_t  m_indexCount{0};
};

} // namespace jade
