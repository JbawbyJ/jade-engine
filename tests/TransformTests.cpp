// Transform tests — pure math, no GL context. Every expectation is either the
// algebraic definition (translation adds, scale multiplies) or is rebuilt in
// the test from glm's own primitives (glm::translate/rotate/scale), so the
// suite pins Transform::matrix() to its documented T · Ry · Rx · Rz · S
// contract instead of to hand-typed constants.

#include "math/Transform.h"

#include <doctest/doctest.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEps = 1e-5f;

void checkVec4(const jade::Vec4& actual, const jade::Vec4& expected, float eps = kEps) {
    CHECK(actual.x == doctest::Approx(expected.x).epsilon(eps));
    CHECK(actual.y == doctest::Approx(expected.y).epsilon(eps));
    CHECK(actual.z == doctest::Approx(expected.z).epsilon(eps));
    CHECK(actual.w == doctest::Approx(expected.w).epsilon(eps));
}

void checkMat4(const jade::Mat4& actual, const jade::Mat4& expected, float eps = kEps) {
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            CAPTURE(col);
            CAPTURE(row);
            CHECK(actual[col][row] == doctest::Approx(expected[col][row]).epsilon(eps));
        }
    }
}

jade::Mat4 rotateY(float radians) {
    return glm::rotate(jade::Mat4{1.0f}, radians, jade::Vec3{0.0f, 1.0f, 0.0f});
}

} // namespace

TEST_CASE("Transform: default matrix() is the identity") {
    const jade::Transform transform;
    checkMat4(transform.matrix(), jade::Mat4{1.0f});

    const jade::Vec4 point{1.2f, -3.4f, 5.6f, 1.0f};
    checkVec4(transform.matrix() * point, point);
}

TEST_CASE("Transform: pure translation moves a point by exactly the offset") {
    jade::Transform transform;
    transform.position = jade::Vec3{3.0f, -2.0f, 5.5f};

    const jade::Vec3 point{0.25f, 4.0f, -1.0f};
    const jade::Vec4 moved = transform.matrix() * jade::Vec4{point, 1.0f};
    checkVec4(moved, jade::Vec4{point + transform.position, 1.0f});
}

TEST_CASE("Transform: pure scale multiplies componentwise") {
    jade::Transform transform;
    transform.scale = jade::Vec3{2.0f, 3.0f, 0.5f};

    const jade::Vec3 point{1.0f, -2.0f, 4.0f};
    const jade::Vec4 scaled = transform.matrix() * jade::Vec4{point, 1.0f};
    checkVec4(scaled, jade::Vec4{point * transform.scale, 1.0f}); // glm vec*vec is componentwise
}

TEST_CASE("Transform: pure yaw +pi/2 turns +X the way glm's Ry does (to -Z)") {
    jade::Transform transform;
    transform.rotationEuler.y = kPi / 2.0f; // yaw only

    const jade::Vec4 unitX{1.0f, 0.0f, 0.0f, 1.0f};
    const jade::Vec4 turned = transform.matrix() * unitX;

    // Reference comes straight from the library, not from hand arithmetic.
    checkVec4(turned, rotateY(kPi / 2.0f) * unitX);

    // Sanity: a real 90° turn in the XZ plane — length kept, y untouched,
    // perpendicular to where it started.
    const jade::Vec3 v{turned};
    CHECK(glm::length(v) == doctest::Approx(1.0f).epsilon(kEps));
    CHECK(v.y == doctest::Approx(0.0f).epsilon(kEps));
    CHECK(glm::dot(v, jade::Vec3{1.0f, 0.0f, 0.0f}) == doctest::Approx(0.0f).epsilon(kEps));

    // In glm's right-handed Y-up convention that lands on -Z (matches the
    // Camera convention: positive yaw swings +X-ward views, -Z forward).
    CHECK(v.x == doctest::Approx(0.0f).epsilon(kEps));
    CHECK(v.z == doctest::Approx(-1.0f).epsilon(kEps));
}

TEST_CASE("Transform: TRS order applies scale first, then rotation, then translation") {
    jade::Transform transform;
    transform.position = jade::Vec3{1.0f, 2.0f, 3.0f};
    transform.rotationEuler.y = kPi / 2.0f;
    transform.scale = jade::Vec3{2.0f, 2.0f, 2.0f};

    const jade::Vec3 point{0.5f, -1.0f, 1.5f};

    // Hand-derived stepwise, innermost first — like nested CSS transforms:
    // scale the point, rotate the scaled point, then add the translation.
    const jade::Vec3 scaled = point * 2.0f;
    const jade::Vec3 rotated{rotateY(kPi / 2.0f) * jade::Vec4{scaled, 1.0f}};
    const jade::Vec3 expected = rotated + transform.position;

    checkVec4(transform.matrix() * jade::Vec4{point, 1.0f}, jade::Vec4{expected, 1.0f});
}

TEST_CASE("Transform: matrix() equals the manual T*Ry*Rx*Rz*S product") {
    jade::Transform transform;
    transform.position = jade::Vec3{-2.0f, 0.5f, 4.0f};
    transform.rotationEuler = jade::Vec3{0.4f, 1.1f, -0.7f}; // pitch, yaw, roll
    transform.scale = jade::Vec3{1.5f, 0.75f, 2.0f};

    const jade::Mat4 translation = glm::translate(jade::Mat4{1.0f}, transform.position);
    const jade::Mat4 yawRot = rotateY(transform.rotationEuler.y);
    const jade::Mat4 pitchRot =
        glm::rotate(jade::Mat4{1.0f}, transform.rotationEuler.x, jade::Vec3{1.0f, 0.0f, 0.0f});
    const jade::Mat4 rollRot =
        glm::rotate(jade::Mat4{1.0f}, transform.rotationEuler.z, jade::Vec3{0.0f, 0.0f, 1.0f});
    const jade::Mat4 scaling = glm::scale(jade::Mat4{1.0f}, transform.scale);

    const jade::Mat4 reference = translation * yawRot * pitchRot * rollRot * scaling;
    checkMat4(transform.matrix(), reference);

    // Rotation order is load-bearing: the naive pitch-then-yaw product moves a
    // probe point somewhere measurably different, so the checks above would
    // catch a swapped Ry/Rx in matrix().
    const jade::Mat4 swapped = translation * pitchRot * yawRot * rollRot * scaling;
    const jade::Vec4 probe{1.0f, 0.0f, 0.0f, 1.0f};
    const jade::Vec3 divergence{reference * probe - swapped * probe};
    CHECK(glm::length(divergence) > 0.1f);
}

// --- interpolate() — Phase 5 snapshot + blend ------------------------------

namespace {

// Two deliberately unrelated full poses; every Euler component stays within
// pi of its partner so the endpoint cases are pure "reproduce from/to" checks
// with no wrap in play.
jade::Transform makeFrom() {
    jade::Transform t;
    t.position = jade::Vec3{1.0f, 2.0f, 3.0f};
    t.rotationEuler = jade::Vec3{0.4f, 1.1f, -0.7f};
    t.scale = jade::Vec3{1.5f, 0.75f, 2.0f};
    return t;
}

jade::Transform makeTo() {
    jade::Transform t;
    t.position = jade::Vec3{-2.0f, 0.5f, 7.0f};
    t.rotationEuler = jade::Vec3{-0.2f, 0.6f, 0.9f};
    t.scale = jade::Vec3{0.5f, 3.0f, 1.0f};
    return t;
}

} // namespace

TEST_CASE("interpolate: alpha 0 reproduces from.matrix(), alpha 1 to.matrix()") {
    const jade::Transform from = makeFrom();
    const jade::Transform to = makeTo();

    // All 16 entries, Approx: alpha 1 walks from + 1*(to - from), and that
    // float round trip may land an ulp or two off `to`'s own components.
    checkMat4(jade::interpolate(from, to, 0.0f), from.matrix());
    checkMat4(jade::interpolate(from, to, 1.0f), to.matrix());
}

TEST_CASE("interpolate: alpha 0.5 blends position and scale to the midpoint") {
    // Rotation stays zero in both poses so a transformed point isolates the
    // translate + scale halves: M * p = midScale * p + midPosition.
    jade::Transform from;
    from.position = jade::Vec3{1.0f, 2.0f, 3.0f};
    from.scale = jade::Vec3{1.0f, 1.0f, 1.0f};

    jade::Transform to;
    to.position = jade::Vec3{3.0f, -2.0f, 7.0f};
    to.scale = jade::Vec3{3.0f, 5.0f, 1.0f};

    // Derived stepwise, same shape as the implementation's lerp.
    const jade::Vec3 midPosition = from.position + 0.5f * (to.position - from.position);
    const jade::Vec3 midScale = from.scale + 0.5f * (to.scale - from.scale);

    const jade::Vec3 point{1.0f, -1.0f, 2.0f};
    const jade::Vec3 expected = midScale * point + midPosition; // scale first, then translate
    checkVec4(jade::interpolate(from, to, 0.5f) * jade::Vec4{point, 1.0f},
              jade::Vec4{expected, 1.0f});
}

TEST_CASE("interpolate: yaw +3.0 to -3.0 blends across the +/-pi seam, not through 0") {
    // Rotation-only poses so the matrix is pure Ry.
    jade::Transform from;
    from.rotationEuler.y = 3.0f;
    jade::Transform to;
    to.rotationEuler.y = -3.0f;

    // Derive the short arc stepwise. The raw difference is -6.0 rad, which is
    // below -pi, so the shortest arc adds one full turn: -6 + 2*pi = +0.2831...
    // — a small POSITIVE step onward past +pi, not the -6.0 march back
    // through 0.
    const float rawDiff = to.rotationEuler.y - from.rotationEuler.y;
    CHECK(rawDiff < -kPi); // precondition that makes this a wrap case at all
    const float wrappedDiff = rawDiff + 2.0f * kPi;
    const float expectedYaw = from.rotationEuler.y + 0.5f * wrappedDiff; // ~= +pi

    // Halfway along the short arc sits on the seam itself: |yaw| ~= pi.
    CHECK(std::fabs(expectedYaw) == doctest::Approx(kPi).epsilon(1e-4));

    jade::Transform expected;
    expected.rotationEuler.y = expectedYaw;
    const jade::Mat4 blended = jade::interpolate(from, to, 0.5f);
    checkMat4(blended, expected.matrix(), 1e-4f);

    // Behavioral check via the forward direction: yaw ~pi turns -Z to +Z.
    // The long way (naive lerp -> yaw 0) would leave forward at -Z, so the z
    // sign alone separates the two arcs.
    const jade::Vec4 forward = blended * jade::Vec4{0.0f, 0.0f, -1.0f, 0.0f};
    CHECK(forward.z == doctest::Approx(1.0f).epsilon(1e-4));

    jade::Transform longWay;
    longWay.rotationEuler.y = from.rotationEuler.y + 0.5f * rawDiff; // = 0.0, through 0
    const jade::Vec4 longForward = longWay.matrix() * jade::Vec4{0.0f, 0.0f, -1.0f, 0.0f};
    CHECK(glm::length(jade::Vec3{forward - longForward}) > 1.0f); // opposite hemispheres
}
