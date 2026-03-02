#pragma once

#include <memory>
#include <span>
#include <vector>

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

	std::span<const SpriteDrawable> buildSprites();

private:
	std::vector<std::unique_ptr<Entity>> m_entities;

	std::vector<SpriteDrawable> m_sprites;
};
