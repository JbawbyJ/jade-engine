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
