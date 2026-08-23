#pragma once

// Wraps every OpenGL call with error reporting. Required by the day-one rule:
// "Write every OpenGL call inside GL_CHECK() macro from day one."
//
// Usage:  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
//
// This drains glGetError() after the call and logs each pending error with
// the originating __FILE__/__LINE__. On 4.3+ contexts (or with KHR_debug)
// installGlDebugCallback() adds push-based driver diagnostics on top;
// GL_CHECK stays on every call as the 3.3-path fallback.

#include <glad/glad.h>

#include "core/Logger.h"

namespace jade {

/// Install the KHR_debug / GL 4.3+ core message callback on the CURRENT
/// context. Push-based: the driver calls us the instant it records a problem
/// (like subscribing to an error event stream instead of polling a status
/// endpoint the way GL_CHECK does). Severity mapping: HIGH -> JADE_LOG_ERROR,
/// MEDIUM/LOW -> JADE_LOG_WARN, NOTIFICATION -> dropped (spam filter).
/// Returns true only when the callback is actually installed; false on
/// contexts without debug output (e.g. the 3.3/4.1 fallback rungs), where
/// GL_CHECK polling remains the only diagnostic. Call after glad has loaded.
bool installGlDebugCallback();

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
