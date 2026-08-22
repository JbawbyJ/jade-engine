// Camera tests — pure math, no GL context. Directions and transformed points
// are checked componentwise with doctest::Approx, since sin/cos and matrix
// chains accumulate float noise.

#include "renderer/Camera.h"

#include <doctest/doctest.h>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEps = 1e-5f;

void checkVec4(const jade::Vec4& actual, const jade::Vec4& expected, float eps = kEps) {
    CHECK(actual.x == doctest::Approx(expected.x).epsilon(eps));
    CHECK(actual.y == doctest::Approx(expected.y).epsilon(eps));
    CHECK(actual.z == doctest::Approx(expected.z).epsilon(eps));
    CHECK(actual.w == doctest::Approx(expected.w).epsilon(eps));
}

} // namespace

TEST_CASE("Camera: default forward faces -Z") {
    const jade::Camera camera;
    const jade::Vec3 fwd = camera.forward();
    CHECK(fwd.x == doctest::Approx(0.0f).epsilon(kEps));
    CHECK(fwd.y == doctest::Approx(0.0f).epsilon(kEps));
    CHECK(fwd.z == doctest::Approx(-1.0f).epsilon(kEps));
}

TEST_CASE("Camera: yaw +pi/2 turns forward to +X") {
    jade::Camera camera;
    camera.setYaw(kPi / 2.0f);
    const jade::Vec3 fwd = camera.forward();
    CHECK(fwd.x == doctest::Approx(1.0f).epsilon(kEps));
    CHECK(fwd.y == doctest::Approx(0.0f).epsilon(kEps));
    CHECK(fwd.z == doctest::Approx(0.0f).epsilon(kEps));
}

TEST_CASE("Camera: pitch just under +pi/2 points nearly straight up") {
    jade::Camera camera;
    camera.setPitch(kPi / 2.0f - 0.001f); // shy of vertical so lookAt's Y-up basis stays sane
    CHECK(camera.forward().y == doctest::Approx(1.0f).epsilon(1e-3f));
}

TEST_CASE("Camera: view() maps the camera position to the origin") {
    jade::Camera camera;
    camera.setPosition(jade::Vec3{3.0f, -1.5f, 7.0f});
    camera.setYaw(0.4f);
    camera.setPitch(-0.2f);

    const jade::Vec4 eye = camera.view() * jade::Vec4{camera.position(), 1.0f};
    checkVec4(eye, jade::Vec4{0.0f, 0.0f, 0.0f, 1.0f});
}

TEST_CASE("Camera: view() maps position + forward to (0, 0, -1)") {
    jade::Camera camera;
    camera.setPosition(jade::Vec3{1.0f, 2.0f, 3.0f});
    camera.setYaw(0.7f);
    camera.setPitch(0.3f);

    const jade::Vec3 ahead = camera.position() + camera.forward();
    const jade::Vec4 viewSpace = camera.view() * jade::Vec4{ahead, 1.0f};
    checkVec4(viewSpace, jade::Vec4{0.0f, 0.0f, -1.0f, 1.0f});
}

TEST_CASE("Camera: viewProjection() equals projection() * view()") {
    jade::Camera camera;
    camera.setPosition(jade::Vec3{0.5f, 1.0f, 4.0f});
    camera.setYaw(-0.3f);
    camera.setPitch(0.1f);

    const jade::Vec4 point{0.3f, -0.2f, -1.5f, 1.0f};
    const jade::Vec4 combined = camera.viewProjection() * point;
    const jade::Vec4 chained = camera.projection() * (camera.view() * point);
    checkVec4(combined, chained);
}

TEST_CASE("Camera: setPerspective changes projection()") {
    jade::Camera camera;
    const jade::Vec4 point{1.0f, 1.0f, -2.0f, 1.0f};
    const jade::Vec4 before = camera.projection() * point;

    camera.setPerspective(kPi / 2.0f, 0.5f, 50.0f); // wider fov, shifted planes
    const jade::Vec4 after = camera.projection() * point;

    // A wider fov shrinks the x/y scale; new near/far remap clip z.
    CHECK(after.x != doctest::Approx(before.x).epsilon(kEps));
    CHECK(after.y != doctest::Approx(before.y).epsilon(kEps));
    CHECK(after.z != doctest::Approx(before.z).epsilon(kEps));
    // Clip w stays -z_view regardless of fov or planes.
    CHECK(after.w == doctest::Approx(2.0f).epsilon(kEps));
}

TEST_CASE("Camera: setAspect rescales clip x by the aspect ratio") {
    jade::Camera camera;
    const jade::Vec4 point{1.0f, 0.5f, -3.0f, 1.0f};
    const jade::Vec4 wide = camera.projection() * point; // default 16:9

    camera.setAspect(4.0f / 3.0f);
    const jade::Vec4 narrow = camera.projection() * point;

    // x scale is 1 / (aspect * tan(fovY/2)), so x_new / x_old = old / new aspect.
    const float expectedRatio = (16.0f / 9.0f) / (4.0f / 3.0f);
    CHECK(narrow.x == doctest::Approx(wide.x * expectedRatio).epsilon(kEps));
    // y is untouched by aspect.
    CHECK(narrow.y == doctest::Approx(wide.y).epsilon(kEps));
}
