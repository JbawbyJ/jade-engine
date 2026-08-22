#include "core/Input.h"

#include <GLFW/glfw3.h>

#include "core/Logger.h"

namespace jade {

namespace {
    // Only these codes are snapshotted for wasKeyPressed / wasKeyReleased.
    // isKeyDown still live-queries any GLFW key so gameplay can pass raw codes.
    constexpr int kTrackedKeys[] = {
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
}

void Input::update() {
    // Shift current → last, then refresh current from GLFW. Edge queries
    // (wasKeyPressed / wasKeyReleased) compare the two snapshots.
    for (int key : kTrackedKeys) {
        const std::size_t i = static_cast<std::size_t>(key);
        m_keysDownLast[i] = m_keysDown[i];
        m_keysDown[i] = glfwGetKey(m_window, key) == GLFW_PRESS;
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
    // Live query — valid for any GLFW key code, not only kTrackedKeys.
    if (!m_window || key < 0) return false;
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::wasKeyPressed(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    return m_keysDown[i] && !m_keysDownLast[i];
}

bool Input::wasKeyReleased(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
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
