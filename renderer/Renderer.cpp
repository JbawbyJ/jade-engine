#include "renderer/Renderer.h"

#include <glad/glad.h>
#include <glm/geometric.hpp>

#include "core/GLDebug.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

namespace jade {

Renderer::Renderer() {
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    m_lightDirection = glm::normalize(m_lightDirection);
}

void Renderer::setClearColor(const Vec4& color) {
    m_clearColor = color;
}

void Renderer::setViewProjection(const Mat4& viewProjection) {
    m_viewProjection = viewProjection;
}

void Renderer::setLightDirection(const Vec3& direction) {
    // Ignore a degenerate zero vector rather than normalizing it into NaNs.
    if (glm::dot(direction, direction) > 0.0f) {
        m_lightDirection = glm::normalize(direction);
    }
}

void Renderer::beginFrame() const {
    GL_CHECK(glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::draw(const Mesh& mesh, const Shader& shader) const {
    shader.bind();
    shader.setMat4("uViewProj", m_viewProjection);
    shader.setMat4("uModel", Mat4{1.0f});
    mesh.draw();
}

void Renderer::draw(const Mesh& mesh, const Shader& shader, const Texture& texture) const {
    draw(mesh, shader, texture, Mat4{1.0f});
}

void Renderer::draw(const Mesh& mesh, const Shader& shader, const Texture& texture,
                    const Mat4& model) const {
    shader.bind();
    texture.bind(0);
    shader.setInt("uTexture", 0);
    shader.setMat4("uViewProj", m_viewProjection);
    shader.setMat4("uModel", model);
    shader.setVec3("uLightDir", m_lightDirection);
    mesh.draw();
}

} // namespace jade
