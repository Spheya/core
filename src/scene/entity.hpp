#pragma once

#include <cstdint>
#include <span>

#include "physics/bounding_box.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_drawable.hpp"
#include "scene.hpp"
#include "time.hpp"

class Entity {
public:
	virtual ~Entity() = default;

	virtual void onUpdate(const Time& time) = 0;
	virtual std::span<const SpriteDrawable> getSprites() const { return {}; }

	[[nodiscard]] BoundingBox getPhysicsBounds() const { return { localPhysicsBounds.min + position, localPhysicsBounds.max + position }; }
	[[nodiscard]] BoundingBox getClickBounds() const { return { localClickBounds.min + position, localClickBounds.max + position }; }

	void setScene(Scene* scene) { this->scene = scene; }
	[[nodiscard]] Scene* getScene() const { return scene; }

	void markForDestruction() { m_markedForDestruction = true; }
	[[nodiscard]] bool isMarkedForDestruction() const { return m_markedForDestruction; }

public:
	uint32_t flags = 0;
	glm::vec2 position = glm::vec2(0.0f);
	BoundingBox localPhysicsBounds;
	BoundingBox localClickBounds;

protected:
	Scene* scene = nullptr;

private:
	bool m_markedForDestruction = false;
};
