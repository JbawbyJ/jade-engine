#pragma once

// First-person style camera: a position plus yaw/pitch (radians) in a Y-up
// world. Pure math — no GL, no GLFW, no Input — so it unit-tests without a
// context. Think of view() as the inverse of a CSS transform: instead of
// moving the camera through the world, it moves the world in front of a
// fixed lens. Yaw 0 / pitch 0 faces −Z; positive yaw turns toward +X.

#include "math/MathTypes.h"

namespace jade {

class Camera {
public:
    Camera() = default;

    // World → view matrix (glm::lookAt from position along forward(), Y up).
    Mat4 view() const;

    // View → clip matrix (glm::perspective with the stored fov/aspect/planes).
    Mat4 projection() const;

    // projection() * view() — the one matrix shaders usually want per frame.
    Mat4 viewProjection() const;

    // Unit direction the camera faces:
    // {cos(pitch)·sin(yaw), sin(pitch), −cos(pitch)·cos(yaw)}.
    Vec3 forward() const;

    Vec3 position() const { return m_position; }
    void setPosition(const Vec3& position) { m_position = position; }

    float yaw() const { return m_yaw; }
    void setYaw(float radians) { m_yaw = radians; }

    float pitch() const { return m_pitch; }
    // Stored verbatim — no clamping. Pitch limits (avoiding gimbal flip at
    // ±90°) are the Phase 5 controller's job; Camera keeps what it is given.
    void setPitch(float radians) { m_pitch = radians; }

    // Call on framebuffer resize (width / height).
    void setAspect(float aspect) { m_aspect = aspect; }

    // Named nearPlane/farPlane on purpose: Windows headers #define near/far.
    void setPerspective(float fovYRadians, float nearPlane, float farPlane);

private:
    static constexpr float kPi = 3.14159265358979323846f;
    static constexpr float kDefaultFovY = kPi / 3.0f; // 60° vertical FOV
    static constexpr float kDefaultAspect = 16.0f / 9.0f;
    static constexpr float kDefaultNear = 0.1f;
    static constexpr float kDefaultFar = 100.0f;
    // A step back along +Z so the origin is in view by default.
    static constexpr Vec3 kDefaultPosition{0.0f, 0.0f, 2.0f};

    Vec3  m_position{kDefaultPosition};
    float m_yaw{0.0f};
    float m_pitch{0.0f};
    float m_fovY{kDefaultFovY};
    float m_aspect{kDefaultAspect};
    float m_near{kDefaultNear};
    float m_far{kDefaultFar};
};

} // namespace jade
