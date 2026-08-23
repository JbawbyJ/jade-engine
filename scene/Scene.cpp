#include "scene/Scene.h"

#include <utility>

namespace jade {

Entity& Scene::createEntity(std::string name) {
    Entity& entity = m_entities.emplace_back();
    entity.name = std::move(name);
    // transform / mesh / texture keep their Entity defaults: identity
    // transform, null (non-owning) mesh + texture.
    return entity;
}

std::vector<Entity>& Scene::entities() {
    return m_entities;
}

const std::vector<Entity>& Scene::entities() const {
    return m_entities;
}

void Scene::snapshotPrevious() {
    for (Entity& entity : m_entities) {
        entity.previousTransform = entity.transform;
    }
}

} // namespace jade
