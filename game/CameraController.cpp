// Movement math for the fly camera. Kept out of the header so includers see
// only forward declarations (standards rule 14); the only real dependencies
// are Camera (output), Input (queries), and <cmath>/<algorithm>.

#include "game/CameraController.h"

#include "core/Input.h"
#include "renderer/Camera.h"

#include <algorithm>
#include <cmath>

namespace jade {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

// Shortest-arc angle blend — same idiom as math/Transform.cpp:
// std::remainder(x, 2*pi) wraps the difference into [-pi, pi], so a yaw
// tween from +179° to -179° swings 2° through 180°, not 358° the long way.
float lerpAngleShortest(float from, float to, float alpha) {
    const float diff = std::remainder(to - from, kTwoPi);
    return from + alpha * diff;
}

} // namespace

CameraController::CameraController(Camera& camera)
    : m_camera(camera),
      m_position(camera.position()),
      m_yaw(camera.yaw()),
      m_pitch(camera.pitch()),
      m_previousPosition(m_position),
      m_previousYaw(m_yaw),
      m_previousPitch(m_pitch) {}

void CameraController::snapshotPrevious() {
    m_previousPosition = m_position;
    m_previousYaw = m_yaw;
    m_previousPitch = m_pitch;
    m_lookAppliedThisFrame = false;
}

void CameraController::fixedUpdate(const Input& input, float dt) {
    // --- Look (latched: once per render frame, not once per fixed step) ---
    // mouseDelta() is pixels since the previous Input::update(), i.e. a
    // per-render-frame delta. Applying it in every banked catch-up step
    // would double/triple the turn on hitches, so only the first step after
    // snapshotPrevious() consumes it. Look runs before movement so this
    // step's WASD travels along the fresh heading.
    if (!m_lookAppliedThisFrame) {
        m_lookAppliedThisFrame = true;
        if (input.isMouseButtonDown(Mouse::Right)) {
            const MousePosition delta = input.mouseDelta();
            m_yaw += static_cast<float>(delta.x) * kLookSpeed;
            // Screen y grows downward, so dragging up (negative dy) looks up.
            m_pitch -= static_cast<float>(delta.y) * kLookSpeed;
            m_pitch = std::clamp(m_pitch, -kMaxPitch, kMaxPitch);
        }
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
    // the classic strafe-running exploit. Opposing keys cancel to ~zero
    // length, which the guard treats as "no input".
    const float planarLength =
        std::sqrt(planar.x * planar.x + planar.z * planar.z);
    if (planarLength > 0.0f) {
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
