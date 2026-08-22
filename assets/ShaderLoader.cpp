#include "assets/ShaderLoader.h"

#include <fstream>
#include <iterator>
#include <string>

#include "assets/AssetError.h"
#include "assets/AssetPaths.h"
#include "core/Logger.h"

namespace jade {

namespace {

    // Slurp a whole text file into a string. Binary mode: no newline
    // translation, so the bytes handed to the GLSL compiler are exactly the
    // bytes on disk. Throws AssetError with the full resolved path when the
    // file is missing, unopenable, or a read error occurs.
    std::string readTextFile(const std::filesystem::path& absolute) {
        std::ifstream file(absolute, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw AssetError("Failed to open shader file: " + absolute.string());
        }

        // istreambuf_iterator reads until EOF; badbit afterwards means a real
        // I/O error (an empty file is fine and yields an empty string).
        std::string contents{std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>()};
        if (file.bad()) {
            throw AssetError("Failed to read shader file: " + absolute.string());
        }
        return contents;
    }

} // namespace

std::unique_ptr<Shader> loadShader(const std::filesystem::path& vertRelative,
                                   const std::filesystem::path& fragRelative) {
    const std::filesystem::path vertPath = assetRoot() / vertRelative;
    const std::filesystem::path fragPath = assetRoot() / fragRelative;

    // File I/O happens fully before any GL work, so an AssetError here never
    // leaves a half-built program behind.
    const std::string vertexSource = readTextFile(vertPath);
    const std::string fragmentSource = readTextFile(fragPath);

    JADE_LOG_INFO("Loading shader: " + vertPath.string() + " + " + fragPath.string());

    // Delegate to the from-memory constructor: compile/link failures keep
    // their existing ShaderError semantics, untouched by the asset layer.
    return std::make_unique<Shader>(vertexSource, fragmentSource);
}

} // namespace jade
