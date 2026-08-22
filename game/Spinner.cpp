#include "game/Spinner.h"

#include "core/Logger.h"

#include <cmath>

namespace jade {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

} // namespace

Spinner::Spinner(float radiansPerSecond)
    : m_radiansPerSecond(radiansPerSecond) {}

void Spinner::add(EntityId id) {
    m_ids.push_back(id);
}

void Spinner::fixedUpdate(Scene& scene, float dt) {
    std::vector<Entity>& entities = scene.entities();
    for (EntityId id : m_ids) {
        // Hard invariant, not a runtime skip: Phase 3 froze "no removal,
        // indices never shift", so an id past the end can only mean the
        // caller registered an entity that was never created.
        JADE_ASSERT(id < entities.size(),
                    "Spinner::fixedUpdate: registered EntityId out of range");

        float& yaw = entities[id].transform.rotationEuler.y;
        yaw += m_radiansPerSecond * dt;
        // Wrap into [-pi, pi]. sin/cos are periodic so rendering never needs
        // this — but left unbounded, hours of spinning push the float into
        // magnitudes where one fixed-step increment falls below its
        // precision and the spin visibly degrades. std::remainder(x, 2*pi)
        // is the exact wrap (x minus the nearest multiple of 2*pi), and
        // interpolate() blends angles shortest-arc, so the seam where yaw
        // jumps from ~+pi to ~-pi is invisible to rendering.
        yaw = std::remainder(yaw, kTwoPi);
    }
}

} // namespace jade
