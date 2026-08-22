#pragma once

// Thin glm aliases used by the renderer (and later by gameplay math).
// glm stays the implementation; call sites include this header instead of
// pulling glm directly — same idea as a TS `type Vec3 = [number, number, number]`
// wrapper so the engine can swap the backing library later without a hunt.

#include <glm/glm.hpp>

namespace jade {

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using IVec2 = glm::ivec2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

} // namespace jade
