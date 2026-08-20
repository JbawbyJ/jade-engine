#pragma once

// RAII wrapper around a GLFW window + an OpenGL 4.6 core context.
// One Window == one OS window == one GL context. The class owns its native
// handle and cleans up GLFW automatically when the last Window is destroyed.

#include <functional>
#include <string>

// Forward-declare so this header doesn't drag <GLFW/glfw3.h> into every
// translation unit that includes it. Faster compiles + cleaner public API.
struct GLFWwindow;

namespace jade {

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

    // Non-copyable: the window owns a unique OS resource. Moves not needed yet.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void pollEvents() const;   // pump OS events into GLFW callbacks
    void swapBuffers() const;  // present the back buffer
    bool shouldClose() const;  // true once the user clicks X or we request close
    void requestClose() const; // ask the loop to exit on the next iteration

    int width()  const { return m_width;  }
    int height() const { return m_height; }
    GLFWwindow* native() const { return m_window; } // escape hatch for GLFW input

    void setResizeCallback(ResizeCallback cb) { m_onResize = std::move(cb); }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow*    m_window{nullptr};
    int            m_width{0};
    int            m_height{0};
    ResizeCallback m_onResize{};
};

} // namespace jade
