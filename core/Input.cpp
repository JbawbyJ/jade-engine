#include "core/Input.h"

#include <GLFW/glfw3.h>

#include "core/Logger.h"

namespace jade {

namespace {
    // Snapshot set seeds: the named Key:: constants are always tracked; any
    // other key auto-enrolls on its first edge query (see wasKeyPressed).
    constexpr int kDefaultTrackedKeys[] = {
        Key::Space,
        Key::A,
        Key::D,
        Key::S,
        Key::W,
        Key::Escape,
        Key::LeftShift,
        Key::LeftCtrl,
        Key::LeftAlt,
    };
}

Input::Input(GLFWwindow* window)
    : m_window(window) {
    JADE_ASSERT(m_window != nullptr, "Input requires a live GLFWwindow*");

    // Sticky mode: a press+release that both land inside one pollEvents call
    // still reads as GLFW_PRESS on the next poll, so per-frame snapshots
    // cannot drop a quick tap between frames.
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(m_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    for (int key : kDefaultTrackedKeys) {
        m_keyTracked[static_cast<std::size_t>(key)] = true;
    }
}

void Input::update() {
    // Shift current → last, then refresh current from GLFW. Edge queries
    // (wasKeyPressed / wasKeyReleased) compare the two snapshots. Scanning
    // the tracked flags is a 512-bool sweep — noise next to a GL frame.
    for (std::size_t i = 0; i < kKeyCount; ++i) {
        if (!m_keyTracked[i]) {
            continue;
        }
        m_keysDownLast[i] = m_keysDown[i];
        m_keysDown[i] = glfwGetKey(m_window, static_cast<int>(i)) == GLFW_PRESS;
    }
    for (std::size_t i = 0; i < kButtonCount; ++i) {
        m_buttonsDownLast[i] = m_buttonsDown[i];
        m_buttonsDown[i] =
            glfwGetMouseButton(m_window, static_cast<int>(i)) == GLFW_PRESS;
    }
}

bool Input::validKey(int key) const {
    return key >= 0 && static_cast<std::size_t>(key) < kKeyCount;
}

bool Input::validButton(int button) const {
    return button >= 0 && static_cast<std::size_t>(button) < kButtonCount;
}

bool Input::isKeyDown(int key) const {
    // Live query — valid for any GLFW key code, not only the tracked set.
    // Note: with sticky mode on, a live poll consumes a pending sticky press,
    // so taps are better read through wasKeyPressed.
    if (!validKey(key)) return false;
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::wasKeyPressed(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    m_keyTracked[i] = true; // enroll: accurate from the next update() onward
    return m_keysDown[i] && !m_keysDownLast[i];
}

bool Input::wasKeyReleased(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    m_keyTracked[i] = true; // enroll: accurate from the next update() onward
    return !m_keysDown[i] && m_keysDownLast[i];
}

bool Input::isMouseButtonDown(int button) const {
    if (!validButton(button)) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

bool Input::wasMouseButtonPressed(int button) const {
    if (!validButton(button)) return false;
    const std::size_t i = static_cast<std::size_t>(button);
    return m_buttonsDown[i] && !m_buttonsDownLast[i];
}

bool Input::wasMouseButtonReleased(int button) const {
    if (!validButton(button)) return false;
    const std::size_t i = static_cast<std::size_t>(button);
    return !m_buttonsDown[i] && m_buttonsDownLast[i];
}

MousePosition Input::mousePosition() const {
    MousePosition pos{};
    glfwGetCursorPos(m_window, &pos.x, &pos.y);
    return pos;
}

} // namespace jade
