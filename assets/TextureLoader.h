#pragma once

// Phase 4 asset loader: decodes an image file with stb_image and uploads it
// into a GPU Texture. Recoverable failures (missing file, corrupt image)
// throw AssetError with the full resolved path — never JADE_ASSERT.

#include <filesystem>
#include <memory>

namespace jade {

// Forward declaration keeps renderer/Texture.h out of includers (standards
// rule 14). Callers need the full renderer/Texture.h wherever the returned
// unique_ptr is used or destroyed.
class Texture;

/// Loads the image at `assetRoot() / relative`, vertically flipped and forced
/// to RGBA8, and uploads it as a Texture. Throws AssetError on decode failure.
/// Returns std::unique_ptr because Texture is non-movable and ownership
/// transfers to the caller — rule 4's smart-pointer allowance.
std::unique_ptr<Texture> loadTexture(const std::filesystem::path& relative);

} // namespace jade
