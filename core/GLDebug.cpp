#include "core/GLDebug.h"

#include <string>

#include "core/Logger.h"

namespace jade {

namespace {
    // Short human-readable names for the callback's source/type enums so a log
    // line reads "GL API ERROR [id=1282]: ..." instead of raw hex. Kept tiny on
    // purpose — this runs inside the driver's callback.
    const char* debugSourceName(GLenum source) {
        switch (source) {
            case GL_DEBUG_SOURCE_API:             return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW_SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER_COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:     return "THIRD_PARTY";
            case GL_DEBUG_SOURCE_APPLICATION:     return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER:           return "OTHER";
            default:                              return "UNKNOWN_SOURCE";
        }
    }

    const char* debugTypeName(GLenum type) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED";
            case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:              return "MARKER";
            case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH_GROUP";
            case GL_DEBUG_TYPE_POP_GROUP:           return "POP_GROUP";
            case GL_DEBUG_TYPE_OTHER:               return "OTHER";
            default:                                return "UNKNOWN_TYPE";
        }
    }

    // The driver invokes this from its own C frames (APIENTRY calling
    // convention), so it follows the same two rules as the GLFW callbacks in
    // Window.cpp: match the C ABI exactly, and never let a C++ exception
    // unwind through the driver (UB) — hence the exception barrier below.
    void APIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length,
                                       const GLchar* message,
                                       const void* userParam) {
        (void)userParam; // we registered nullptr; nothing to recover

        // Drop NOTIFICATION chatter (buffer usage hints, etc.) — the push
        // stream is only valuable if real problems are not buried in spam.
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            return;
        }

        // Exception barrier: same rule as Window's resize callback — logging
        // allocates, and an exception must not escape into driver frames.
        try {
            // KHR_debug: `length` is the message size when non-negative;
            // otherwise the string is null-terminated.
            std::string text = std::string("GL ") + debugSourceName(source)
                               + " " + debugTypeName(type)
                               + " [id=" + std::to_string(id) + "]: ";
            if (message != nullptr) {
                if (length >= 0) {
                    text.append(message, static_cast<std::size_t>(length));
                } else {
                    text.append(message);
                }
            }

            if (severity == GL_DEBUG_SEVERITY_HIGH) {
                JADE_LOG_ERROR(text);
            } else {
                // MEDIUM and LOW both land on WARN — actionable, not fatal.
                JADE_LOG_WARN(text);
            }
        } catch (const std::exception& error) {
            JADE_LOG_ERROR(std::string("GL debug callback threw: ") + error.what());
        } catch (...) {
            JADE_LOG_ERROR("GL debug callback threw a non-standard exception");
        }
    }

    // Does the current context offer debug output at all? Core since 4.3;
    // older contexts may still expose it via the KHR_debug extension.
    bool contextSupportsDebugOutput() {
        // glad records the negotiated version in its global GLVersion struct
        // when the loader runs.
        if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 3)) {
            return true;
        }
        // Extension probe. Guarded because not every glad flavor ships
        // extension support: our local dev glad (pip, all extensions) guards
        // the KHR_debug block with the GL_KHR_debug macro and exposes the
        // runtime flag as the int GLAD_GL_KHR_debug; CI's vcpkg glad (no
        // extensions feature) omits the whole block, so referencing the int
        // unguarded would not compile there.
#if defined(GL_KHR_debug)
        return GLAD_GL_KHR_debug != 0;
#else
        return false;
#endif
    }
} // namespace

bool installGlDebugCallback() {
    if (!contextSupportsDebugOutput()) {
        return false;
    }

    // Belt and braces: even when the header declares the entry point, the
    // loader leaves the function POINTER null on a context that does not
    // actually provide it (e.g. 4.1 without the extension). Calling through
    // null would crash, so bail to the GL_CHECK-only path instead.
    if (glDebugMessageCallback == nullptr) {
        return false;
    }

    GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
    // Synchronous delivery: the callback fires on the offending thread before
    // the GL call returns, so a debugger backtrace points at the real caller.
    // Slower, but this is a diagnostics path — clarity wins over throughput.
    GL_CHECK(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    GL_CHECK(glDebugMessageCallback(&debugMessageCallback, nullptr));

    // TODO(jade): use glDebugMessageControl to mute known-noisy message ids
    //             per driver once we meet them in the wild.
    return true;
}

} // namespace jade
