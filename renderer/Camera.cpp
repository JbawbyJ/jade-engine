// Camera math lives here so the header stays glm-core only: the transform
// helpers (lookAt / perspective) come from gtc, which call sites never see.

#include "renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace jade {

Vec3 Camera::forward() const {
    // Spherical → Cartesian with the −Z-at-rest convention: yaw sweeps around
    // the Y axis (positive toward +X), pitch tilts off the horizon.
    const float cosPitch = std::cos(m_pitch);
    return Vec3{
        cosPitch * std::sin(m_yaw),
        std::sin(m_pitch),
        -cosPitch * std::cos(m_yaw),
    };
}

Mat4 Camera::view() const {
    return glm::lookAt(m_position, m_position + forward(), Vec3{0.0f, 1.0f, 0.0f});
}

Mat4 Camera::projection() const {
    return glm::perspective(m_fovY, m_aspect, m_near, m_far);
}

Mat4 Camera::viewProjection() const {
    return projection() * view();
}

void Camera::setPerspective(float fovYRadians, float nearPlane, float farPlane) {
    m_fovY = fovYRadians;
    m_near = nearPlane;
    m_far = farPlane;
}

} // namespace jade
