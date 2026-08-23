#include "renderer/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <cstddef>
#include <string>

#include "core/GLDebug.h"
#include "core/Logger.h"

namespace jade {

namespace {
    std::string shaderStageName(unsigned int type) {
        switch (type) {
            case GL_VERTEX_SHADER:   return "vertex";
            case GL_FRAGMENT_SHADER: return "fragment";
            default:                 return "unknown";
        }
    }

    std::string fetchShaderLog(unsigned int shader) {
        int length = 0;
        GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));
        if (length <= 1) {
            return {};
        }
        std::string log(static_cast<std::size_t>(length), '\0');
        GL_CHECK(glGetShaderInfoLog(shader, length, nullptr, log.data()));
        while (!log.empty() && log.back() == '\0') {
            log.pop_back();
        }
        return log;
    }

    std::string fetchProgramLog(unsigned int program) {
        int length = 0;
        GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
        if (length <= 1) {
            return {};
        }
        std::string log(static_cast<std::size_t>(length), '\0');
        GL_CHECK(glGetProgramInfoLog(program, length, nullptr, log.data()));
        while (!log.empty() && log.back() == '\0') {
            log.pop_back();
        }
        return log;
    }

    unsigned int compileStage(unsigned int type, std::string_view source) {
        unsigned int shader = 0;
        GL_CHECK(shader = glCreateShader(type));
        if (shader == 0) {
            throw ShaderError("glCreateShader returned 0");
        }

        const char* src = source.data();
        const int length = static_cast<int>(source.size());
        GL_CHECK(glShaderSource(shader, 1, &src, &length));
        GL_CHECK(glCompileShader(shader));

        int success = 0;
        GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if (success != GL_TRUE) {
            const std::string log = fetchShaderLog(shader);
            const std::string message = std::string("Failed to compile ")
                + shaderStageName(type) + " shader: " + log;
            JADE_LOG_ERROR(message);
            GL_CHECK(glDeleteShader(shader));
            throw ShaderError(message);
        }
        return shader;
    }
}

Shader::Shader(std::string_view vertexSource, std::string_view fragmentSource) {
    const unsigned int vert = compileStage(GL_VERTEX_SHADER, vertexSource);
    unsigned int frag = 0;
    try {
        frag = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
    } catch (...) {
        GL_CHECK(glDeleteShader(vert));
        throw;
    }

    GL_CHECK(m_id = glCreateProgram());
    if (m_id == 0) {
        GL_CHECK(glDeleteShader(vert));
        GL_CHECK(glDeleteShader(frag));
        throw ShaderError("glCreateProgram returned 0");
    }

    GL_CHECK(glAttachShader(m_id, vert));
    GL_CHECK(glAttachShader(m_id, frag));
    GL_CHECK(glLinkProgram(m_id));

    int success = 0;
    GL_CHECK(glGetProgramiv(m_id, GL_LINK_STATUS, &success));
    if (success != GL_TRUE) {
        const std::string log = fetchProgramLog(m_id);
        const std::string message = std::string("Failed to link shader program: ") + log;
        JADE_LOG_ERROR(message);
        GL_CHECK(glDeleteShader(vert));
        GL_CHECK(glDeleteShader(frag));
        GL_CHECK(glDeleteProgram(m_id));
        m_id = 0;
        throw ShaderError(message);
    }

    // Stages are baked into the program; delete the objects like discarding
    // transpile intermediates after a successful JS bundle.
    GL_CHECK(glDetachShader(m_id, vert));
    GL_CHECK(glDetachShader(m_id, frag));
    GL_CHECK(glDeleteShader(vert));
    GL_CHECK(glDeleteShader(frag));
}

Shader::~Shader() {
    if (m_id != 0) {
        GL_CHECK(glDeleteProgram(m_id));
        m_id = 0;
    }
}

void Shader::bind() const {
    GL_CHECK(glUseProgram(m_id));
}

void Shader::unbind() const {
    GL_CHECK(glUseProgram(0));
}

int Shader::uniformLocation(const char* name) const {
    // Cached: glGetUniformLocation is a driver-side string lookup, and draw
    // paths hit setters every frame. Misses are cached as -1 with one warning
    // so a typo'd or optimized-out name is loud exactly once, not silent.
    const auto found = m_uniformLocations.find(name);
    if (found != m_uniformLocations.end()) {
        return found->second;
    }

    int location = -1;
    GL_CHECK(location = glGetUniformLocation(m_id, name));
    m_uniformLocations.emplace(name, location);
    if (location < 0) {
        JADE_LOG_WARN(std::string("Uniform '") + name
                      + "' not active (unused/optimized out, or misspelled)");
    }
    return location;
}

void Shader::setInt(const char* name, int value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniform1i(location, value));
}

void Shader::setFloat(const char* name, float value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniform1f(location, value));
}

void Shader::setVec2(const char* name, const Vec2& value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniform2f(location, value.x, value.y));
}

void Shader::setVec3(const char* name, const Vec3& value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniform3f(location, value.x, value.y, value.z));
}

void Shader::setVec4(const char* name, const Vec4& value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniform4f(location, value.x, value.y, value.z, value.w));
}

void Shader::setMat4(const char* name, const Mat4& value) const {
    bind();
    const int location = uniformLocation(name);
    if (location < 0) {
        return;
    }
    GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)));
}

} // namespace jade
