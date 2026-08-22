#pragma once

// Keyboard + mouse query facade over GLFW.
// Call update() once per frame after Window::pollEvents() — same rhythm as
// reading `event.key` after a browser event loop tick, then asking
// "is Escape down?" without touching the DOM (GLFW) from every call site.

#include <cstddef>

// Forward-declare so this header does not pull <GLFW/glfw3.h> into every TU.
struct GLFWwindow;

namespace jade {

// GLFW key / button codes mirrored as named constants so call sites do not
// need to include GLFW. Values match GLFW 3.x (Escape == GLFW_KEY_ESCAPE).
namespace Key {
    constexpr int Space      = 32;
    constexpr int A          = 65;
    constexpr int D          = 68;
    constexpr int S          = 83;
    constexpr int W          = 87;
    constexpr int Escape     = 256;
    constexpr int LeftShift  = 340;
    constexpr int LeftCtrl   = 341;
    constexpr int LeftAlt    = 342;
    // TODO(jade): expand as gameplay bindings land
}

namespace Mouse {
    constexpr int Left   = 0;
    constexpr int Right  = 1;
    constexpr int Middle = 2;
}

struct MousePosition {
    double x{0.0};
    double y{0.0};
};

class Input {
public:
    explicit Input(GLFWwindow* window);
    ~Input() = default;

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    // Snapshot GLFW state into this frame / last frame buffers.
    // Must run after Window::pollEvents() each frame.
    void update();

    bool isKeyDown(int key) const;
    bool wasKeyPressed(int key) const;   // down this frame, up last frame
    bool wasKeyReleased(int key) const;  // up this frame, down last frame

    bool isMouseButtonDown(int button) const;
    bool wasMouseButtonPressed(int button) const;
    bool wasMouseButtonReleased(int button) const;

    MousePosition mousePosition() const;

private:
    static constexpr std::size_t kKeyCount    = 512; // covers GLFW_KEY_LAST (348)
    static constexpr std::size_t kButtonCount = 8;   // covers GLFW_MOUSE_BUTTON_LAST

    bool validKey(int key) const;
    bool validButton(int button) const;

    GLFWwindow* m_window{nullptr};

    bool m_keysDown[kKeyCount]{};
    bool m_keysDownLast[kKeyCount]{};
    bool m_buttonsDown[kButtonCount]{};
    bool m_buttonsDownLast[kButtonCount]{};
};

} // namespace jade
