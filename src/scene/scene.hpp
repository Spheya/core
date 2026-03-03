#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "physics/bounding_box.hpp"
#include "physics/intersection.hpp"
#include "physics/window_physics.hpp"
#include "rendering/sprite_drawable.hpp"
#include "time.hpp"

class Entity;

class Scene {
public:
	Scene() = default;
	Scene(Scene&) = delete;
	Scene& operator=(Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;
	~Scene() = default;

	Entity* addEntity(std::unique_ptr<Entity> entity);

	void update(const Time& time);
	void addWindowPhysics(const WindowPhysics* windowPhysics);

	[[nodiscard]] bool overlaps(const BoundingBox& box, uint32_t flags = ~0u) const;
	[[nodiscard]] Intersection rayCast(
	    glm::vec2 origin, glm::vec2 direction, float maxDistance = 1e32f, uint32_t flags = ~0u, const Entity* exclude = nullptr,
	    bool includeWindows = true
	) const;
	[[nodiscard]] Intersection boxCast(
	    const BoundingBox& origin, glm::vec2 direction, float maxDistance = 1e32f, uint32_t flags = ~0u, const Entity* exclude = nullptr,
	    bool includeWindows = true
	) const;

	std::span<const SpriteDrawable> buildSprites();

private:
	std::vector<std::unique_ptr<Entity>> m_entities;
	const WindowPhysics* m_windowPhysics = nullptr;

	std::vector<SpriteDrawable> m_sprites;
	std::vector<BoundingBox> m_clickRegions;
};
