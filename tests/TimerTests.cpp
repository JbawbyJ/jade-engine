// Deterministic Timer tests. Everything drives advance() — the injected-dt
// seam — so no test ever sleeps or reads the wall clock.

#include "core/Timer.h"

#include <doctest/doctest.h>

TEST_CASE("Timer: fixedDelta is 1/60") {
    const jade::Timer timer;
    CHECK(timer.fixedDelta() == 1.0f / 60.0f);
}

TEST_CASE("Timer: fresh timer has nothing to consume") {
    jade::Timer timer;
    CHECK_FALSE(timer.consumeFixedStep());
    CHECK(timer.deltaTime() == 0.0f);
    CHECK(timer.totalTime() == 0.0);
}

TEST_CASE("Timer: advancing below one step banks no consumable step") {
    jade::Timer timer;
    timer.advance(0.01f); // < 1/60, so the accumulator stays under one step
    CHECK_FALSE(timer.consumeFixedStep());
}

TEST_CASE("Timer: advancing exactly one step yields one step, then none") {
    jade::Timer timer;
    timer.advance(1.0f / 60.0f);
    CHECK(timer.consumeFixedStep());
    CHECK_FALSE(timer.consumeFixedStep());
}

TEST_CASE("Timer: oversized advance clamps to 0.25s and yields 15 steps") {
    jade::Timer timer;
    timer.advance(0.5f); // clamped to kMaxDelta = 0.25 → 0.25 / (1/60) = 15
    int steps = 0;
    while (timer.consumeFixedStep()) {
        ++steps;
    }
    CHECK(steps == 15);
}

TEST_CASE("Timer: totalTime accumulates clamped deltas across advances") {
    jade::Timer timer;
    timer.advance(0.1f);
    timer.advance(0.5f); // clamped to 0.25
    timer.advance(0.2f);
    CHECK(timer.totalTime() == doctest::Approx(0.55).epsilon(0.0001));
}

TEST_CASE("Timer: deltaTime reflects the clamped value") {
    jade::Timer timer;
    timer.advance(1.0f);
    CHECK(timer.deltaTime() == 0.25f); // kMaxDelta, exactly representable
    timer.advance(0.02f);
    CHECK(timer.deltaTime() == doctest::Approx(0.02f)); // last advance wins
}

TEST_CASE("Timer: fresh timer has alpha 0") {
    const jade::Timer timer;
    CHECK(timer.alpha() == 0.0f); // empty accumulator, exactly zero
}

TEST_CASE("Timer: half a fixed step gives alpha ~0.5 and no consumable step") {
    jade::Timer timer;
    // 0.5f * fixedDelta scales the float by a power of two, so the
    // accumulator holds exactly half a step and alpha divides back out.
    timer.advance(0.5f * timer.fixedDelta());
    CHECK(timer.alpha() == doctest::Approx(0.5f).epsilon(1e-6));
    CHECK_FALSE(timer.consumeFixedStep()); // half a step banks nothing
}

TEST_CASE("Timer: alpha returns to ~0 after a one-step advance is drained") {
    jade::Timer timer;
    timer.advance(timer.fixedDelta());
    CHECK(timer.consumeFixedStep());
    // Same float added then subtracted — the residue is zero, but assert
    // "near zero" so a future reformulation of the drain stays legal.
    CHECK(timer.alpha() == doctest::Approx(0.0f).epsilon(1e-6));
    CHECK_FALSE(timer.consumeFixedStep());
}

TEST_CASE("Timer: alpha stays in [0, 1) after any single advance + drain") {
    // Post-drain the accumulator is < kFixedDelta by construction, so alpha
    // must sit in [0, 1) — that is the interpolation contract Phase 5 renders
    // with. Sweep spans below, at, above, and beyond the clamp.
    const float spans[] = {0.0f,          0.01f, 1.0f / 60.0f, 0.1f,
                           1.0f / 60.0f * 7.5f,  0.25f,        0.9f};
    for (const float rawSeconds : spans) {
        CAPTURE(rawSeconds);
        jade::Timer timer;
        timer.advance(rawSeconds);
        while (timer.consumeFixedStep()) {
        }
        CHECK(timer.alpha() >= 0.0f);
        CHECK(timer.alpha() < 1.0f);
    }
}
