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
    return key >= kFirstValidKey && key <= kLastValidKey;
}

void Input::enrollKey(std::size_t index) const {
    if (m_keyTracked[index]) {
        return;
    }
    // Seed BOTH snapshots from the live state: a key that is already held at
    // enrollment must not read as a fresh press edge on the next frame.
    // Poll twice: the first poll consumes a pending sticky press, the second
    // reports the true live state. Seeding from the second avoids both a
    // phantom press edge (key genuinely held) and an orphan release edge
    // (sticky tap consumed into a true seed that clears next frame).
    m_keyTracked[index] = true;
    (void)glfwGetKey(m_window, static_cast<int>(index));
    const bool down =
        glfwGetKey(m_window, static_cast<int>(index)) == GLFW_PRESS;
    m_keysDown[index] = down;
    m_keysDownLast[index] = down;
}

bool Input::validButton(int button) const {
    return button >= 0 && static_cast<std::size_t>(button) < kButtonCount;
}

bool Input::isKeyDown(int key) const {
    // Tracked keys answer from this frame's snapshot: every query in a frame
    // agrees, and a sticky sub-frame tap is not silently consumed by a live
    // poll. Untracked keys fall back to a live query (and stay untracked —
    // only the edge queries opt a key into snapshotting).
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    if (m_keyTracked[i]) {
        return m_keysDown[i];
    }
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::wasKeyPressed(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    enrollKey(i); // accurate from the next update() onward
    return m_keysDown[i] && !m_keysDownLast[i];
}

bool Input::wasKeyReleased(int key) const {
    if (!validKey(key)) return false;
    const std::size_t i = static_cast<std::size_t>(key);
    enrollKey(i); // accurate from the next update() onward
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
