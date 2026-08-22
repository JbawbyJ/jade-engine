// Sanity checks over the glm aliases. Intentionally small — this TU exists to
// prove the aliases compile and glm links into the test build, not to re-test
// glm itself.

#include "math/MathTypes.h"

#include <doctest/doctest.h>

TEST_CASE("Math: Vec3 arithmetic is componentwise") {
    const jade::Vec3 a{1.0f, 2.0f, 3.0f};
    const jade::Vec3 b{4.0f, 5.0f, 6.0f};

    const jade::Vec3 sum = a + b;
    CHECK(sum.x == 5.0f);
    CHECK(sum.y == 7.0f);
    CHECK(sum.z == 9.0f);

    const jade::Vec3 scaled = a * 2.0f;
    CHECK(scaled.x == 2.0f);
    CHECK(scaled.y == 4.0f);
    CHECK(scaled.z == 6.0f);
}

TEST_CASE("Math: Mat4 identity leaves a vector unchanged") {
    const jade::Mat4 identity{1.0f};
    const jade::Vec4 v{1.0f, -2.0f, 3.5f, 1.0f};

    const jade::Vec4 out = identity * v;
    CHECK(out.x == 1.0f);
    CHECK(out.y == -2.0f);
    CHECK(out.z == 3.5f);
    CHECK(out.w == 1.0f);
}

TEST_CASE("Math: IVec2 equality is exact per component") {
    CHECK(jade::IVec2{800, 600} == jade::IVec2{800, 600});
    CHECK(jade::IVec2{800, 600} != jade::IVec2{800, 601});
}
