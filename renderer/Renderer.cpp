#include "renderer/Renderer.h"

#include <glad/glad.h>

#include "core/GLDebug.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

namespace jade {

Renderer::Renderer() {
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}

void Renderer::setClearColor(const Vec4& color) {
    m_clearColor = color;
}

void Renderer::setViewProjection(const Mat4& viewProjection) {
    m_viewProjection = viewProjection;
}

void Renderer::beginFrame() const {
    GL_CHECK(glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::draw(const Mesh& mesh, const Shader& shader) const {
    shader.bind();
    shader.setMat4("uViewProj", m_viewProjection);
    mesh.draw();
}

void Renderer::draw(const Mesh& mesh, const Shader& shader, const Texture& texture) const {
    shader.bind();
    texture.bind(0);
    shader.setInt("uTexture", 0);
    shader.setMat4("uViewProj", m_viewProjection);
    mesh.draw();
}

} // namespace jade
