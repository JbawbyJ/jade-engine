#pragma once

// Loads a shader program from two GLSL text files under assetRoot().
// File problems throw AssetError; compile/link problems keep throwing
// ShaderError from the Shader constructor — the loader only does file I/O.

#include <filesystem>
#include <memory>

// Full Shader definition (not a forward declaration): every caller receives a
// unique_ptr<Shader> and needs the complete type to use — and to destroy — it.
#include "renderer/Shader.h"

namespace jade {

// Both paths are relative to assetRoot(), e.g. "shaders/basic.vert".
// Ownership transfers to the caller via unique_ptr (rule 4 allowance: Shader
// is a non-movable GL RAII type).
// Throws AssetError (missing/unreadable file, message carries the full
// resolved path) or ShaderError (bad GLSL).
std::unique_ptr<Shader> loadShader(const std::filesystem::path& vertRelative,
                                   const std::filesystem::path& fragRelative);

} // namespace jade
