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
    constexpr int F1         = 290; // Phase 6 binding target (== GLFW_KEY_F1)
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

    // Edge queries compare this frame's snapshot against last frame's. A key
    // joins the snapshot set the first time it is queried, so the very first
    // query of a new key reports false and is accurate from the next frame on
    // (sticky input mode keeps sub-frame taps from slipping between polls).
    // Prefer these over isKeyDown for taps.
    bool wasKeyPressed(int key) const;   // down this frame, up last frame
    bool wasKeyReleased(int key) const;  // up this frame, down last frame

    bool isMouseButtonDown(int button) const;
    bool wasMouseButtonPressed(int button) const;
    bool wasMouseButtonReleased(int button) const;

    // Cursor position as of the last update() — a frame-coherent snapshot,
    // not a live poll (changed from live glfwGetCursorPos when mouseDelta
    // landed), matching the tracked-key isKeyDown philosophy: every query
    // inside one frame sees the same value.
    MousePosition mousePosition() const;

    // Pixels the cursor moved since the previous update() (current − last).
    // Zero on the first frame: the constructor seeds both snapshots from the
    // live cursor, so a huge synthetic first-frame delta cannot spin the
    // Phase 5 camera controller.
    MousePosition mouseDelta() const;

private:
    static constexpr std::size_t kKeyCount    = 512; // covers GLFW_KEY_LAST (348)
    static constexpr std::size_t kButtonCount = 8;   // covers GLFW_MOUSE_BUTTON_LAST

    // GLFW's accepted key-token range (GLFW_KEY_SPACE..GLFW_KEY_LAST). Codes
    // outside it make glfwGetKey raise GLFW_INVALID_ENUM, so validKey rejects
    // them up front — one bad query must not turn into per-frame error spam
    // via auto-enrollment.
    static constexpr int kFirstValidKey = 32;
    static constexpr int kLastValidKey  = 348;

    bool validKey(int key) const;
    bool validButton(int button) const;

    GLFWwindow* m_window{nullptr};

    void enrollKey(std::size_t index) const;

    // The key-tracking arrays are mutable so const edge queries can lazily
    // enroll a key into the snapshot set (a cache decision, not logical
    // state); enrollment seeds both snapshots from the live state so an
    // already-held key cannot manufacture a phantom press edge. Seeded with
    // the Key:: constants in the constructor.
    mutable bool m_keyTracked[kKeyCount]{};
    mutable bool m_keysDown[kKeyCount]{};
    mutable bool m_keysDownLast[kKeyCount]{};
    bool m_buttonsDown[kButtonCount]{};
    bool m_buttonsDownLast[kButtonCount]{};

    // Cursor snapshots: current as of the latest update(), last as of the
    // update() before it. Seeded equal in the constructor (zero first delta).
    MousePosition m_mousePosition{};
    MousePosition m_mousePositionLast{};
};

} // namespace jade
