#pragma once

// Flat entity list — Phase 3 decision: no hierarchy. The demo end state
// (camera over a handful of meshes) never needs parenting, and
// Transform::matrix() keeps that seam open for later.
//
// Storage is one contiguous std::vector, so an EntityId is just an index
// into it. IMPORTANT: the Entity& returned by createEntity() INVALIDATES on
// the next createEntity() (vector reallocation moves the elements) — hold an
// EntityId across spawns, not a reference. Same trap as keeping a live
// pointer into a JS array you keep pushing to, except C++ makes it a
// dangling reference instead of a stale copy.

#include "scene/Entity.h"

#include <cstddef>
#include <string>
#include <vector>

namespace jade {

// Index into Scene::entities(). Stable across createEntity() calls
// (Phase 3 never removes entities, so indices never shift).
using EntityId = std::size_t;

class Scene {
public:
    // Append a default-constructed entity (identity transform, null mesh /
    // texture) carrying the moved-in name. The returned reference is valid
    // only until the next createEntity() — see the invalidation note above.
    Entity& createEntity(std::string name);

    // Direct access to the flat list; iterate it to draw.
    std::vector<Entity>& entities();
    const std::vector<Entity>& entities() const;

    // Copy every entity's current transform into previousTransform. Call once
    // per render frame, before draining fixed steps, so interpolation always
    // blends between the two most recent simulation states.
    void snapshotPrevious();

    // TODO(jade): removal + generation ids when gameplay needs them.

private:
    std::vector<Entity> m_entities;
};

} // namespace jade
