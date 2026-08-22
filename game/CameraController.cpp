// Movement math for the fly camera. Kept out of the header so includers see
// only forward declarations (standards rule 14); the only real dependencies
// are Camera (output), Input (queries), and <cmath>/<algorithm>.

#include "game/CameraController.h"

#include "core/Input.h"
#include "math/Transform.h" // lerpAngleShortest — one definition, shared
#include "renderer/Camera.h"

#include <algorithm>
#include <cmath>

namespace jade {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

// Below this, planar input is treated as "no keys": opposing keys cancel to
// rounding noise, and normalizing that noise would inflate it to full-speed
// drift. The epsilon keeps correctness independent of evaluation order.
constexpr float kPlanarDeadzone = 1e-4f;

} // namespace

CameraController::CameraController(Camera& camera)
    : m_camera(camera),
      m_position(camera.position()),
      m_yaw(camera.yaw()),
      m_pitch(camera.pitch()),
      m_previousPosition(m_position),
      m_previousYaw(m_yaw),
      m_previousPitch(m_pitch) {}

void CameraController::collectLook(const Input& input) {
    // TODO(jade): capture the cursor (GLFW_CURSOR_DISABLED + raw motion via
    //             Window) so a long drag cannot stall at the screen edge.
    if (input.isMouseButtonDown(Mouse::Right)) {
        const MousePosition delta = input.mouseDelta();
        m_pendingLook.x += static_cast<float>(delta.x);
        m_pendingLook.y += static_cast<float>(delta.y);
    }
}

void CameraController::snapshotPrevious() {
    m_previousPosition = m_position;
    m_previousYaw = m_yaw;
    m_previousPitch = m_pitch;
}

void CameraController::fixedUpdate(const Input& input, float dt) {
    // --- Look: consume the banked per-frame delta exactly once ---
    // The bank zeroes on consumption, so a catch-up frame's extra steps add
    // nothing, and motion banked on zero-step frames waits here instead of
    // evaporating (mouse sensitivity must not scale with framerate). Look
    // runs before movement so this step's WASD travels the fresh heading.
    if (m_pendingLook.x != 0.0f || m_pendingLook.y != 0.0f) {
        m_yaw += m_pendingLook.x * kLookSpeed;
        // Screen y grows downward, so dragging up (negative dy) looks up.
        m_pitch -= m_pendingLook.y * kLookSpeed;
        m_pitch = std::clamp(m_pitch, -kMaxPitch, kMaxPitch);
        // Wrap like the Spinner: unbounded yaw degrades float precision over
        // long sessions, and shortest-arc blending hides the ±pi seam.
        m_yaw = std::remainder(m_yaw, kTwoPi);
        m_pendingLook = {0.0f, 0.0f};
    }

    // --- Movement ---
    // Planar basis from yaw alone: Camera::forward() with pitch zeroed is
    // {sin(yaw), 0, -cos(yaw)}; right is that rotated -90° about Y. Pitch is
    // deliberately ignored so W walks along the horizon instead of flying
    // the camera into the ground when looking down.
    const Vec3 forward{std::sin(m_yaw), 0.0f, -std::cos(m_yaw)};
    const Vec3 right{std::cos(m_yaw), 0.0f, std::sin(m_yaw)};

    Vec3 planar{0.0f};
    if (input.isKeyDown(Key::W)) { planar += forward; }
    if (input.isKeyDown(Key::S)) { planar -= forward; }
    if (input.isKeyDown(Key::D)) { planar += right; }
    if (input.isKeyDown(Key::A)) { planar -= right; }

    // Normalize combined directions so W+D is not sqrt(2) faster than W —
    // the classic strafe-running exploit. Opposing keys cancel to rounding
    // noise; the deadzone treats that as "no input" instead of normalizing
    // noise up to full speed.
    const float planarLength =
        std::sqrt(planar.x * planar.x + planar.z * planar.z);
    if (planarLength > kPlanarDeadzone) {
        m_position += planar * (kMoveSpeed * dt / planarLength);
    }

    float vertical = 0.0f;
    if (input.isKeyDown(Key::Space)) { vertical += 1.0f; }
    if (input.isKeyDown(Key::LeftShift)) { vertical -= 1.0f; }
    m_position.y += vertical * kMoveSpeed * dt;
}

void CameraController::writeToCamera(float alpha) const {
    m_camera.setPosition(
        m_previousPosition + alpha * (m_position - m_previousPosition));
    m_camera.setYaw(lerpAngleShortest(m_previousYaw, m_yaw, alpha));
    m_camera.setPitch(lerpAngleShortest(m_previousPitch, m_pitch, alpha));
}

} // namespace jade
