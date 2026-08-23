#include "math/Transform.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace jade {

Mat4 Transform::matrix() const {
    // T · Ry · Rx · Rz · S — glm::rotate/translate/scale each post-multiply,
    // so applying them in this order builds exactly that product.
    Mat4 m{1.0f};
    m = glm::translate(m, position);
    m = glm::rotate(m, rotationEuler.y, Vec3{0.0f, 1.0f, 0.0f});
    m = glm::rotate(m, rotationEuler.x, Vec3{1.0f, 0.0f, 0.0f});
    m = glm::rotate(m, rotationEuler.z, Vec3{0.0f, 0.0f, 1.0f});
    m = glm::scale(m, scale);
    return m;
}

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

} // namespace

// Shortest-arc blend of one angle: wrap the difference into [-pi, pi], then
// walk that far from `from`. std::remainder(x, 2*pi) is exactly that wrap —
// it returns x minus the nearest multiple of 2*pi. Same trick as normalizing
// a compass-heading tween in JS so 350° → 10° animates through 0°, not 180°.
float lerpAngleShortest(float from, float to, float alpha) {
    const float diff = std::remainder(to - from, kTwoPi);
    return from + alpha * diff;
}

Mat4 interpolate(const Transform& from, const Transform& to, float alpha) {
    // Build the blended pose, then reuse Transform::matrix() so the compose
    // order (T · Ry · Rx · Rz · S) has exactly one definition and can never
    // drift out of sync with the un-blended path.
    Transform blended;
    blended.position = from.position + alpha * (to.position - from.position);
    blended.scale = from.scale + alpha * (to.scale - from.scale);
    // Per-component Euler shortest-arc blend — see the header for the
    // documented limitation versus a true quaternion slerp.
    blended.rotationEuler.x = lerpAngleShortest(from.rotationEuler.x, to.rotationEuler.x, alpha);
    blended.rotationEuler.y = lerpAngleShortest(from.rotationEuler.y, to.rotationEuler.y, alpha);
    blended.rotationEuler.z = lerpAngleShortest(from.rotationEuler.z, to.rotationEuler.z, alpha);
    return blended.matrix();
}

} // namespace jade
