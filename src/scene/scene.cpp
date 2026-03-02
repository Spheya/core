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
		BoundingBox clickBounds = e->getClickBounds();
		BoundingBox physicsBounds = e->getPhysicsBounds();
		if(clickBounds.min != clickBounds.max) GraphicsContext::getInstance().getDebugRenderer().box(clickBounds, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		if(physicsBounds.min != physicsBounds.max)
			GraphicsContext::getInstance().getDebugRenderer().box(physicsBounds, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
#endif
}

std::span<const SpriteDrawable> Scene::buildSprites() {
	m_sprites.clear();

	for(auto& e : m_entities) m_sprites.append_range(e->getSprites());

	return m_sprites;
}
