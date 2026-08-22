#pragma once

// One named thing in the scene: a transform plus what to draw it with.
//
// OWNERSHIP RULE: `mesh` and `texture` are NON-OWNING pointers. The Entity
// never allocates, frees, or outlives them — lifetimes belong to the creator
// (main.cpp today, asset loaders in Phase 4). Think of an Entity as a DOM
// node holding a reference to a shared stylesheet: destroying the node must
// not destroy the stylesheet. Null pointers simply mean "nothing to draw yet".
//
// Mesh and Texture are forward-declared so scene headers never pull renderer
// headers (standards rule 14); only code that actually draws includes those.

#include "math/Transform.h"

#include <string>

namespace jade {

class Mesh;
class Texture;

struct Entity {
    std::string    name;
    Transform      transform;
    const Mesh*    mesh{nullptr};
    const Texture* texture{nullptr};
};

} // namespace jade
