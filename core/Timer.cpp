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

    m_delta = elapsed.count();
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
