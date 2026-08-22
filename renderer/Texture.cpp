#include "renderer/Texture.h"

#include <glad/glad.h>

#include "core/GLDebug.h"
#include "core/Logger.h"

namespace jade {

Texture::Texture(int width, int height, const unsigned char* rgba)
    : m_width(width), m_height(height) {
    JADE_ASSERT(width > 0 && height > 0, "Texture size must be positive");
    JADE_ASSERT(rgba != nullptr, "Texture pixels must not be null");

    GL_CHECK(glGenTextures(1, &m_id));
    JADE_ASSERT(m_id != 0, "glGenTextures failed");

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));

    // Nearest + clamp: honest 1x1 / solid colors stay pixel-exact, no wrap bleed.
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    // RGBA rows are 4-byte aligned already, but odd RGB widths would not be.
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, rgba));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture() {
    if (m_id != 0) {
        GL_CHECK(glDeleteTextures(1, &m_id));
        m_id = 0;
    }
}

void Texture::bind(unsigned int unit) const {
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
}

void Texture::unbind() const {
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

} // namespace jade
