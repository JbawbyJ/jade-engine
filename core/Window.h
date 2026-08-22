#pragma once

// RAII wrapper around a GLFW window + an OpenGL core context.
// Prefers 4.6 core, then falls back through 4.5 → 4.3 → 3.3. The hard
// minimum is 3.3 core. One Window == one OS window == one GL context.
//
// Non-copyable and non-movable (Rule of Five): the window owns a unique OS
// resource, and GLFW's user-pointer plus the process-wide init refcount
// would be left inconsistent if the object were transferred.

#include <functional>
#include <stdexcept>
#include <string>

// Forward-declare so this header doesn't drag <GLFW/glfw3.h> into every
// translation unit that includes it. Faster compiles + cleaner public API.
struct GLFWwindow;

namespace jade {

// Recoverable window / context init failure. Callers should catch this
// instead of expecting JADE_ASSERT to abort the process.
class WindowError : public std::runtime_error {
public:
    explicit WindowError(const std::string& message)
        : std::runtime_error(message) {}
};

struct WindowProps {
    std::string title{"Jade Engine"};
    int width{1280};
    int height{720};
};

class Window {
public:
    // Called whenever the framebuffer is resized (in pixels, not screen units).
    // Mirrors a "resize" event emitter in JS-land.
    using ResizeCallback = std::function<void(int width, int height)>;

    explicit Window(const WindowProps& props = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void pollEvents() const;   // pump OS events into GLFW callbacks
    void swapBuffers() const;  // present the back buffer
    bool shouldClose() const;  // true once the user clicks X or we request close
    void requestClose();       // ask the loop to exit on the next iteration

    // Window size in screen coordinates (logical units on Hi-DPI).
    int width()  const { return m_width;  }
    int height() const { return m_height; }

    // Framebuffer size in pixels. Drive glViewport from these, never width()/height().
    int framebufferWidth()  const { return m_framebufferWidth;  }
    int framebufferHeight() const { return m_framebufferHeight; }

    GLFWwindow* native() const { return m_window; } // escape hatch for GLFW input

    void setResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);

    void acquireGlfw();
    void releaseGlfw();

    GLFWwindow*    m_window{nullptr};
    int            m_width{0};
    int            m_height{0};
    int            m_framebufferWidth{0};
    int            m_framebufferHeight{0};
    ResizeCallback m_onResize{};
    bool           m_glfwHeld{false};
};

} // namespace jade
