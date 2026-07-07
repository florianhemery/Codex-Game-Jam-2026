#include "engine/core/world.h"

namespace racer::engine {

entt::entity World::CreateEntity() {
    return registry_.create();
}

void World::DestroyEntity(entt::entity entity) {
    registry_.destroy(entity);
}

} // namespace racer::engine
