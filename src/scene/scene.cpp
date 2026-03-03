#include "scene.hpp"
#include "entity.hpp"

Entity* Scene::addEntity(std::unique_ptr<Entity> entity) {
	m_entities.push_back(std::move(entity));
	m_entities.back()->setScene(this);
	return m_entities.back().get();
}

void Scene::update(const Time& time) {
	for(auto& e : m_entities) e->onUpdate(time);

	for(auto it = m_entities.begin(); it != m_entities.end();) {
		if((*it)->isMarkedForDestruction()) {
			*it = std::move(m_entities.back());
			m_entities.pop_back();
			if(m_entities.empty()) break;
		} else {
			++it;
		}
	}

#ifdef _DEBUG
	for(auto& e : m_entities) {
		BoundingBox physicsBounds = e->getPhysicsBounds();
		if(physicsBounds.min != physicsBounds.max)
			GraphicsContext::getInstance().getDebugRenderer().box(physicsBounds, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
#endif
}

void Scene::addWindowPhysics(const WindowPhysics* windowPhysics) {
	m_windowPhysics = windowPhysics;
}

bool Scene::overlaps(const BoundingBox& box, uint32_t flags) const {
	for(const auto& entity : m_entities) {
		if((entity->flags & flags) == 0) continue;
		auto bounds = entity->getPhysicsBounds();
		if(::overlaps(box, bounds)) return true;
	}
	return false;
}

Intersection Scene::rayCast(
    glm::vec2 origin, glm::vec2 direction, float maxDistance, uint32_t flags, const Entity* exclude, bool includeWindows
) const {
	Intersection hit{ .distance = maxDistance, .normal = glm::vec2(0.0f) };

	for(const auto& entity : m_entities) {
		if(entity.get() == exclude || (entity->flags & flags) == 0) continue;
		auto bounds = entity->getPhysicsBounds();
		auto newHit = ::rayCast(origin, direction, bounds, hit.distance, RayCastExclude::Exit);
		if(newHit.distance != hit.distance) hit = newHit;
	}

	if(includeWindows && m_windowPhysics) {
		auto newHit = m_windowPhysics->rayCast(origin, direction, hit.distance);
		if(newHit.distance != hit.distance) hit = newHit;
	}

	return hit;
}

Intersection Scene::boxCast(
    const BoundingBox& origin, glm::vec2 direction, float maxDistance, uint32_t flags, const Entity* exclude, bool includeWindows
) const {
	Intersection hit{ .distance = maxDistance, .normal = glm::vec2(0.0f) };

	for(const auto& entity : m_entities) {
		if(entity.get() == exclude || (entity->flags & flags) == 0) continue;
		auto bounds = entity->getPhysicsBounds();
		auto newHit = ::boxCast(origin, direction, bounds, hit.distance, RayCastExclude::Exit);
		if(newHit.distance != hit.distance) hit = newHit;
	}

	if(includeWindows && m_windowPhysics) {
		auto newHit = m_windowPhysics->boxCast(origin, direction, hit.distance);
		if(newHit.distance != hit.distance) hit = newHit;
	}

	return hit;
}

std::span<const SpriteDrawable> Scene::buildSprites() {
	m_sprites.clear();

	for(auto& e : m_entities) m_sprites.append_range(e->getSprites());

	return m_sprites;
}
