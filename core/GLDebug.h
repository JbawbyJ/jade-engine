#pragma once

// Wraps every OpenGL call with error reporting. Required by the day-one rule:
// "Write every OpenGL call inside GL_CHECK() macro from day one."
//
// Usage:  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
//
// This drains glGetError() after the call and logs each pending error with
// the originating __FILE__/__LINE__. In Phase 2 we will swap this for
// glDebugMessageCallback (KHR_debug) which is push-based and richer.

#include <glad/glad.h>

#include "core/Logger.h"

namespace jade {

inline void glCheckError(const char* file, int line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* name = "UNKNOWN_GL_ERROR";
        switch (err) {
            case GL_INVALID_ENUM:                  name = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 name = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             name = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 name = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            default: break;
        }
        Logger::log(LogLevel::Error, file, line, name);
    }
}

} // namespace jade

#define GL_CHECK(stmt)                              \
    do {                                            \
        stmt;                                       \
        ::jade::glCheckError(__FILE__, __LINE__);   \
    } while (0)
