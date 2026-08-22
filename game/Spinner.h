#pragma once

// Spins registered entities around Y at a constant rate under the fixed
// timestep. This class exists to PROVE the Phase 5 pipeline: motion simulated
// at 1/60 in fixedUpdate(), rendered through
// interpolate(previousTransform, transform, Timer::alpha()), must look
// visibly smooth at any refresh rate. If a spinning cube stutters, the
// snapshot/blend wiring is wrong — the spinner itself is deliberately trivial.
//
// Called explicitly from the main drain loop (no System base class — Phase 5
// decision: a registry of virtual systems is mini-ECS creep at demo scale).

#include "scene/Scene.h"

#include <vector>

namespace jade {

class Spinner {
public:
    // Positive rate = counter-clockwise yaw when viewed from +Y (right-hand
    // rule around the Y axis).
    explicit Spinner(float radiansPerSecond);

    // Register an entity index to spin. NOT deduplicated on purpose — adding
    // the same id twice makes it spin at double rate. Keeping it a plain
    // append keeps the class trivial; call sites just add each id once.
    void add(EntityId id);

    // Advance rotationEuler.y of every registered entity by rate * dt, then
    // wrap into [-pi, pi]. An out-of-range id is a programmer error
    // (JADE_ASSERT): Phase 3 froze "no removal, indices never shift", so a
    // registered id can never legitimately go stale.
    void fixedUpdate(Scene& scene, float dt);

private:
    float m_radiansPerSecond{0.0f};
    std::vector<EntityId> m_ids;
};

} // namespace jade
