#include "core/Timer.h"

namespace jade {

Timer::Timer()
    : m_last(Clock::now()) {
}

void Timer::tick() {
    const auto now = Clock::now();

    if (!m_started) {
        // First tick establishes the baseline; report a zero delta so callers
        // do not see a huge spike from process start → first frame.
        m_started = true;
        m_last = now;
        m_delta = 0.0f;
        return;
    }

    const std::chrono::duration<float> elapsed = now - m_last;
    m_last = now;

    advance(elapsed.count());
}

void Timer::advance(float rawSeconds) {
    // Pure state-advance step, split out of tick() so tests and replays can
    // drive the timer deterministically. Deliberately leaves m_last and
    // m_started alone — only tick() owns the wall-clock baseline.
    m_delta = rawSeconds;
    if (m_delta > kMaxDelta) {
        m_delta = kMaxDelta;
    }

    m_accumulator += m_delta;
    m_total += static_cast<double>(m_delta);
}

bool Timer::consumeFixedStep() {
    if (m_accumulator < kFixedDelta) {
        return false;
    }
    m_accumulator -= kFixedDelta;
    return true;
}

} // namespace jade
