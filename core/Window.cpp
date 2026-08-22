#include "core/Window.h"

// IMPORTANT: glad MUST be included before GLFW. GLFW will pull in <GL/gl.h>
// otherwise, and you'll get redefinition errors.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <mutex>
#include <string>

#include "core/GLDebug.h"
#include "core/Logger.h"

namespace jade {

namespace {
    // Reference-counted GLFW init. We support multiple Window instances even
    // though Phase 1 only creates one, because tools/editors will need it.
    // The mutex only keeps the count itself coherent; it is NOT a license to
    // construct Windows off the main thread — GLFW requires glfwInit,
    // glfwTerminate, and glfwCreateWindow to run on the main thread.
    // Increment happens only after glfwInit succeeds.
    std::mutex s_glfwMutex;
    int        s_glfwRefCount = 0;

    // 4.1 sits between 4.3 and 3.3 because it is the macOS core-profile
    // ceiling; without that rung macOS would silently fall through to 3.3.
    constexpr int kContextFallbacks[][2] = {
        {4, 6},
        {4, 5},
        {4, 3},
        {4, 1},
        {3, 3},
    };

    void glfwErrorCallback(int code, const char* description) {
        // Exception barrier: this is called from GLFW's C frames, and a C++
        // exception unwinding through them is undefined behavior. Logging
        // allocates, so guard it; if even logging fails there is nothing
        // safe left to do but swallow.
        try {
            JADE_LOG_ERROR(std::string("GLFW error ") + std::to_string(code) + ": " + description);
        } catch (...) {
        }
    }

    const char* glStringOrUnknown(GLenum name) {
        const GLubyte* value = nullptr;
        GL_CHECK(value = glGetString(name));
        if (value == nullptr) {
            return "unknown";
        }
        return reinterpret_cast<const char*>(value);
    }

    GLFWwindow* createWindowWithFallback(int width, int height, const char* title) {
        const int last = static_cast<int>(sizeof(kContextFallbacks) / sizeof(kContextFallbacks[0])) - 1;
        for (int i = 0; i <= last; ++i) {
            const int major = kContextFallbacks[i][0];
            const int minor = kContextFallbacks[i][1];

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
            glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#if !defined(NDEBUG)
            // Debug contexts emit verbose driver diagnostics; only enable in dev builds.
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

            GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (window != nullptr) {
                JADE_LOG_INFO(std::string("Negotiated OpenGL ")
                              + std::to_string(major) + "." + std::to_string(minor)
                              + " core context");
                return window;
            }

            JADE_LOG_WARN(std::string("OpenGL ")
                          + std::to_string(major) + "." + std::to_string(minor)
                          + (i == last ? " core unavailable"
                                       : " core unavailable, trying older..."));
        }
        return nullptr;
    }
}

void Window::acquireGlfw() {
    std::lock_guard<std::mutex> lock(s_glfwMutex);
    if (s_glfwRefCount == 0) {
        glfwSetErrorCallback(glfwErrorCallback);
        if (glfwInit() != GLFW_TRUE) {
            throw WindowError("glfwInit failed");
        }
    }
    ++s_glfwRefCount;
    m_glfwHeld = true;
}

void Window::releaseGlfw() {
    if (!m_glfwHeld) {
        return;
    }
    std::lock_guard<std::mutex> lock(s_glfwMutex);
    m_glfwHeld = false;
    if (s_glfwRefCount > 0 && --s_glfwRefCount == 0) {
        glfwTerminate();
    }
}

Window::Window(const WindowProps& props)
    : m_width(props.width), m_height(props.height) {
    try {
        acquireGlfw();

        // Prefer 4.6 core, then walk down to the documented 3.3 minimum.
        m_window = createWindowWithFallback(m_width, m_height, props.title.c_str());
        if (m_window == nullptr) {
            throw WindowError(
                "Failed to create a window: no OpenGL 3.3+ core context is available");
        }

        // A GL context is bound to a thread, not a process. Make it current here.
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // vsync on; flip to 0 later when profiling

        // Stash `this` on the GLFWwindow so static C callbacks can recover the
        // owning Window* without a global. This is GLFW's idiomatic pattern.
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, &Window::framebufferSizeCallback);
        glfwSetWindowSizeCallback(m_window, &Window::windowSizeCallback);

        // Load OpenGL function pointers via GLAD. This must happen AFTER we have
        // a current context - before this call, every gl* function is nullptr.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            throw WindowError("Failed to load OpenGL via GLAD");
        }

        JADE_LOG_INFO(std::string("OpenGL ")
                      + glStringOrUnknown(GL_VERSION)
                      + " | GPU: "
                      + glStringOrUnknown(GL_RENDERER));

        // Hi-DPI (e.g. Retina) decouples window size from framebuffer size.
        glfwGetWindowSize(m_window, &m_width, &m_height);
        glfwGetFramebufferSize(m_window, &m_framebufferWidth, &m_framebufferHeight);
        GL_CHECK(glViewport(0, 0, m_framebufferWidth, m_framebufferHeight));
    } catch (...) {
        if (m_window != nullptr) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        releaseGlfw();
        throw;
    }
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    releaseGlfw();
}

void Window::pollEvents() const {
    // Drains the OS event queue into GLFW. Triggers our registered callbacks.
    glfwPollEvents();
}

void Window::swapBuffers() const {
    // Presents the back buffer to the screen (front/back double-buffering).
    glfwSwapBuffers(m_window);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::requestClose() {
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Recover our Window* from the user pointer set in the constructor.
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    self->m_framebufferWidth  = width;
    self->m_framebufferHeight = height;

    // Multi-window: this callback can fire while another context is current,
    // so switch, update the viewport, and restore what was current before —
    // hijacking the caller's context would be a silent state change.
    GLFWwindow* previous = glfwGetCurrentContext();
    glfwMakeContextCurrent(window);
    GL_CHECK(glViewport(0, 0, width, height));

    // Exception barrier: user callbacks must not unwind GLFW's C frames (UB).
    try {
        if (self->m_onResize) self->m_onResize(width, height);
    } catch (const std::exception& error) {
        JADE_LOG_ERROR(std::string("Resize callback threw: ") + error.what());
    } catch (...) {
        JADE_LOG_ERROR("Resize callback threw a non-standard exception");
    }

    if (previous != window) {
        glfwMakeContextCurrent(previous); // nullptr is legal: no context current
    }
}

void Window::windowSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    self->m_width  = width;
    self->m_height = height;
}

} // namespace jade
