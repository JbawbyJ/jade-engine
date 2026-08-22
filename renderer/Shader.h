#pragma once

// RAII OpenGL shader program. Compiles vertex + fragment GLSL from memory
// (embedded strings are the Phase 1 path so CI does not need files on disk).
// Compile / link failures are recoverable: they log and throw ShaderError.
// Do not JADE_ASSERT on bad GLSL — that is content, not a programmer invariant.

#include <stdexcept>
#include <string>
#include <string_view>

#include "math/MathTypes.h"

namespace jade {

class ShaderError : public std::runtime_error {
public:
    explicit ShaderError(const std::string& message)
        : std::runtime_error(message) {}
};

class Shader {
public:
    Shader(std::string_view vertexSource, std::string_view fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

    void bind() const;
    void unbind() const;

    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;
    void setVec2(const char* name, const Vec2& value) const;
    void setVec3(const char* name, const Vec3& value) const;
    void setVec4(const char* name, const Vec4& value) const;
    void setMat4(const char* name, const Mat4& value) const;

    unsigned int id() const { return m_id; }

private:
    int uniformLocation(const char* name) const;

    unsigned int m_id{0};
};

} // namespace jade
