#pragma once

// Frame clock + fixed-timestep accumulator.
// Call tick() once per frame, then drain consumeFixedStep() for simulation
// updates while rendering uses the variable deltaTime() — the classic
// "fixed update / variable render" split (same idea as a JS game loop that
// accumulates dt and steps physics at 1/60 while requestAnimationFrame paints).

#include <chrono>

namespace jade {

class Timer {
public:
    Timer();

    // Advance the clock. Call once at the start of each frame.
    void tick();

    // Advance frame state by a raw elapsed span without touching the wall
    // clock: clamps to kMaxDelta, sets deltaTime(), and feeds the fixed-step
    // accumulator and totalTime(). This is the deterministic seam for tests
    // and replays — like injecting a fake dt into a JS game loop instead of
    // reading performance.now(). Real frames still come from tick(), which
    // measures the span and delegates here.
    void advance(float rawSeconds);

    // Seconds since the previous tick (clamped to avoid spiral-of-death).
    float deltaTime() const { return m_delta; }

    // Constant simulation step size in seconds (1/60).
    float fixedDelta() const { return kFixedDelta; }

    // If the accumulator has at least one fixed step banked, consume it and
    // return true. Drive: while (timer.consumeFixedStep()) { /* fixed update */ }
    bool consumeFixedStep();

    // Seconds since the first tick (sum of clamped per-frame deltas).
    double totalTime() const { return m_total; }

private:
    using Clock = std::chrono::steady_clock;

    static constexpr float kFixedDelta = 1.0f / 60.0f;
    // Cap a single frame's contribution so a long hitch cannot explode the
    // accumulator and lock the loop into catching up forever.
    static constexpr float kMaxDelta = 0.25f;

    Clock::time_point m_last{};
    float             m_delta{0.0f};
    float             m_accumulator{0.0f};
    double            m_total{0.0};
    bool              m_started{false};
};

} // namespace jade
