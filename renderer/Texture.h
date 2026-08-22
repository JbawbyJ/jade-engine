#pragma once

// Minimal RAII 2D texture. Uploads tightly packed RGBA8 pixels and binds
// to a texture unit. Phase 1 does not load images from disk (Phase 4).

namespace jade {

class Texture {
public:
    // `rgba` is width * height * 4 bytes, row-major, unpadded RGBA8.
    Texture(int width, int height, const unsigned char* rgba);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(Texture&&) = delete;

    void bind(unsigned int unit = 0) const;
    void unbind() const;

    unsigned int id() const { return m_id; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    unsigned int m_id{0};
    int          m_width{0};
    int          m_height{0};
};

} // namespace jade
