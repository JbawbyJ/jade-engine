#pragma once

// Position / rotation / scale for one entity, composed into a model matrix.
// Euler angles (radians) on purpose at demo scale — simpler to author and
// debug than quaternions; if gimbal problems ever bite, swap the rotation
// representation behind matrix() without touching call sites.
// Compose order: T · Ry(yaw) · Rx(pitch) · Rz(roll) · S — like nesting CSS
// transforms translate() rotateY() rotateX() rotateZ() scale(), innermost
// applied to the mesh first.

#include "math/MathTypes.h"

namespace jade {

struct Transform {
    Vec3 position{0.0f, 0.0f, 0.0f};
    Vec3 rotationEuler{0.0f, 0.0f, 0.0f}; // radians: x = pitch, y = yaw, z = roll
    Vec3 scale{1.0f, 1.0f, 1.0f};

    // Model matrix: T * Ry * Rx * Rz * S. Implemented in the .cpp so
    // glm/gtc stays out of every includer (standards rule 14).
    Mat4 matrix() const;
};

// Blend two poses for snapshot-interpolated rendering (Phase 5): position and
// scale lerp componentwise; each Euler angle blends along the SHORTEST arc
// (the difference is wrapped into [-pi, pi] before from + alpha * diff), then
// the blended TRS composes exactly like Transform::matrix().
// Limitation (documented on purpose): the angles blend per component, not as
// one rotation — a quaternion slerp would blend the combined orientation. At
// demo rates the per-step angle deltas are tiny, so this is indistinguishable.
Mat4 interpolate(const Transform& from, const Transform& to, float alpha);

} // namespace jade
