#include "assets/TextureLoader.h"

// This is the engine's single stb_image implementation TU: the macro below
// expands the library's function bodies here, exactly once. Every other file
// must include <stb_image.h> without defining STB_IMAGE_IMPLEMENTATION.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>

#include "assets/AssetError.h"
#include "assets/AssetPaths.h"
#include "core/Logger.h"
#include "renderer/Texture.h"

namespace jade {

namespace {

// RAII owner for the CPU-side pixel buffer stb_image hands back (rule 4:
// C-library handles live in RAII wrappers). The destructor runs on every
// exit path, so the buffer is freed even if the Texture constructor throws
// through a GL wrapper — like a `finally` around the upload.
class StbPixels {
public:
    explicit StbPixels(stbi_uc* pixels) : m_pixels(pixels) {}
    ~StbPixels() {
        if (m_pixels != nullptr) {
            stbi_image_free(m_pixels);
        }
    }

    StbPixels(const StbPixels&) = delete;
    StbPixels& operator=(const StbPixels&) = delete;

    const stbi_uc* get() const { return m_pixels; }

private:
    stbi_uc* m_pixels{nullptr};
};

} // namespace

std::unique_ptr<Texture> loadTexture(const std::filesystem::path& relative) {
    const std::filesystem::path fullPath = assetRoot() / relative;

    // Image files store rows top-down; GL's UV origin is bottom-left. Flip on
    // load so texture coordinates behave without per-mesh UV surgery.
    stbi_set_flip_vertically_on_load(true);

    int width = 0;
    int height = 0;
    int channelsInFile = 0; // informational only — we force RGBA below
    StbPixels pixels(
        stbi_load(fullPath.string().c_str(), &width, &height, &channelsInFile, 4));
    if (pixels.get() == nullptr) {
        const char* reason = stbi_failure_reason();
        throw AssetError("Failed to load texture '" + fullPath.string() + "': " +
                         (reason != nullptr ? reason : "unknown stb_image error"));
    }

    // unique_ptr, not a value: Texture is non-movable and ownership transfers
    // to the caller (rule 4 allows smart pointers for ownership transfer).
    // The forced 4th channel guarantees the tightly packed RGBA8 layout the
    // Texture constructor documents.
    auto texture = std::make_unique<Texture>(width, height, pixels.get());

    JADE_LOG_INFO("Loaded texture '" + fullPath.string() + "' (" +
                  std::to_string(width) + "x" + std::to_string(height) + ")");
    return texture;
}

} // namespace jade
