#include "renderer/Mesh.h"

#include <glad/glad.h>

#include <cstddef>

#include "core/GLDebug.h"
#include "core/Logger.h"

namespace jade {

Mesh::Mesh(const Vertex* vertices, std::size_t vertexCount,
           const std::uint32_t* indices, std::size_t indexCount)
    : m_indexCount(indexCount) {
    JADE_ASSERT(vertices != nullptr, "Mesh vertices must not be null");
    JADE_ASSERT(indices != nullptr, "Mesh indices must not be null");
    JADE_ASSERT(vertexCount > 0, "Mesh needs at least one vertex");
    JADE_ASSERT(indexCount > 0, "Mesh needs at least one index");

    // VAO records the attribute recipe (like a prepared WebGL vertex state).
    // Bind it first so the EBO binding is stored on the VAO, not globally.
    GL_CHECK(glGenVertexArrays(1, &m_vao));
    GL_CHECK(glGenBuffers(1, &m_vbo));
    GL_CHECK(glGenBuffers(1, &m_ebo));
    JADE_ASSERT(m_vao != 0 && m_vbo != 0 && m_ebo != 0, "Failed to allocate mesh buffers");

    GL_CHECK(glBindVertexArray(m_vao));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
                          static_cast<GLsizeiptr>(vertexCount * sizeof(Vertex)),
                          vertices,
                          GL_STATIC_DRAW));

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                          static_cast<GLsizeiptr>(indexCount * sizeof(std::uint32_t)),
                          indices,
                          GL_STATIC_DRAW));

    const auto stride = static_cast<GLsizei>(sizeof(Vertex));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                                   reinterpret_cast<const void*>(offsetof(Vertex, position))));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                                   reinterpret_cast<const void*>(offsetof(Vertex, color))));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                                   reinterpret_cast<const void*>(offsetof(Vertex, texCoord))));
    GL_CHECK(glEnableVertexAttribArray(2));

    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

Mesh::~Mesh() {
    if (m_vao != 0) {
        GL_CHECK(glDeleteVertexArrays(1, &m_vao));
        m_vao = 0;
    }
    if (m_vbo != 0) {
        GL_CHECK(glDeleteBuffers(1, &m_vbo));
        m_vbo = 0;
    }
    if (m_ebo != 0) {
        GL_CHECK(glDeleteBuffers(1, &m_ebo));
        m_ebo = 0;
    }
}

void Mesh::bind() const {
    GL_CHECK(glBindVertexArray(m_vao));
}

void Mesh::unbind() const {
    GL_CHECK(glBindVertexArray(0));
}

void Mesh::draw() const {
    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES,
                            static_cast<GLsizei>(m_indexCount),
                            GL_UNSIGNED_INT,
                            nullptr));
    GL_CHECK(glBindVertexArray(0));
}

} // namespace jade
