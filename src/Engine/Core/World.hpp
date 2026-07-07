/*
** EPITECH PROJECT, 2026
** racer
** File description:
** entt registry wrapper for ECS
*/

#ifndef WORLD_HPP_
#define WORLD_HPP_

#include <utility>

#include <entt/entt.hpp>

namespace racer::engine {

class World {
public:
    entt::entity CreateEntity();
    void DestroyEntity(entt::entity entity);

    entt::registry &Registry()
    {
        return registry_;
    }

    const entt::registry &Registry() const
    {
        return registry_;
    }

    template <typename T, typename... Args>
    decltype(auto) Add(entt::entity entity, Args &&...args)
    {
        return registry_.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    decltype(auto) Get(entt::entity entity)
    {
        return registry_.get<T>(entity);
    }

    template <typename T>
    decltype(auto) Get(entt::entity entity) const
    {
        return registry_.get<T>(entity);
    }

    template <typename T>
    bool Has(entt::entity entity) const
    {
        return registry_.all_of<T>(entity);
    }

private:
    entt::registry registry_;
};

} // namespace racer::engine

#endif /* !WORLD_HPP_ */
