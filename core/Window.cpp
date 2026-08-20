#include "core/Window.h"

// IMPORTANT: glad MUST be included before GLFW. GLFW will pull in <GL/gl.h>
// otherwise, and you'll get redefinition errors.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "core/GLDebug.h"
#include "core/Logger.h"

namespace jade {

namespace {
    // Reference-counted GLFW init. We support multiple Window instances even
    // though Phase 1 only creates one, because tools/editors will need it.
    int s_glfwRefCount = 0;

    void glfwErrorCallback(int code, const char* description) {
        JADE_LOG_ERROR(std::string("GLFW error ") + std::to_string(code) + ": " + description);
    }
}

Window::Window(const WindowProps& props)
    : m_width(props.width), m_height(props.height) {

    // Initialize GLFW once, regardless of how many Windows exist.
    if (s_glfwRefCount++ == 0) {
        glfwSetErrorCallback(glfwErrorCallback);
        JADE_ASSERT(glfwInit() == GLFW_TRUE, "glfwInit failed");
    }

    // Request an OpenGL 4.6 core profile context. "Core" means deprecated
    // fixed-function APIs are unavailable - only the modern programmable
    // pipeline. Forward-compat removes anything marked deprecated in 4.6.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#if !defined(NDEBUG)
    // Debug contexts emit verbose driver diagnostics; only enable in dev builds.
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(m_width, m_height, props.title.c_str(), nullptr, nullptr);
    JADE_ASSERT(m_window != nullptr, "glfwCreateWindow failed");

    // A GL context is bound to a thread, not a process. Make it current here.
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync on; flip to 0 later when profiling

    // Stash `this` on the GLFWwindow so static C callbacks can recover the
    // owning Window* without a global. This is GLFW's idiomatic pattern.
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, &Window::framebufferSizeCallback);

    // Load OpenGL function pointers via GLAD. This must happen AFTER we have
    // a current context - before this call, every gl* function is nullptr.
    JADE_ASSERT(gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)),
                "Failed to load OpenGL via GLAD");

    JADE_LOG_INFO(std::string("OpenGL ")
                  + reinterpret_cast<const char*>(glGetString(GL_VERSION))
                  + " | GPU: "
                  + reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    // Hi-DPI (e.g. Retina) decouples window size from framebuffer size. Always
    // set the viewport from the framebuffer size, never from the window size.
    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(m_window, &fbW, &fbH);
    GL_CHECK(glViewport(0, 0, fbW, fbH));
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (--s_glfwRefCount == 0) {
        glfwTerminate();
    }
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

void Window::requestClose() const {
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Recover our Window* from the user pointer set in the constructor.
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    self->m_width  = width;
    self->m_height = height;
    GL_CHECK(glViewport(0, 0, width, height));
    if (self->m_onResize) self->m_onResize(width, height);
}

} // namespace jade
