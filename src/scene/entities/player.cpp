#include "player.hpp"
#include "rendering/sprite_atlas.hpp"
#include "rendering/surface_manager.hpp"

Player::Player() {
	Sprite runAnimSprites[] = {
		SpriteAtlas::getInstance().get("player_run_1.png"), SpriteAtlas::getInstance().get("player_run_2.png"),
		SpriteAtlas::getInstance().get("player_run_3.png"), SpriteAtlas::getInstance().get("player_run_4.png"),
		SpriteAtlas::getInstance().get("player_run_5.png"), SpriteAtlas::getInstance().get("player_run_6.png"),
	};

	m_animation = Animation(runAnimSprites, 24, 2, 0);
	localPhysicsBounds = { .min = glm::vec2(-10.0f, 16.0f), .max = glm::vec2(10.0f, 48.0f) };
}

void Player::onUpdate(const Time& time) {
	BoundingBox clickBounds = { .min = glm::vec2(-32.0f, -8.0f) + position, .max = glm::vec2(32.0f, 48.0f) + position };
#ifdef _DEBUG
	GraphicsContext::getInstance().getDebugRenderer().box(clickBounds);
#endif

	if(SurfaceManager::getInstance().canPushClickableRegion()) SurfaceManager::getInstance().pushClickableRegion(clickBounds);

	m_animation.update(time);
	m_sprite.sprite = m_animation.getCurrentFrame();
	m_sprite.matrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)), glm::vec3(96.0f, 96.0f, 1.0f));
}

std::span<const SpriteDrawable> Player::getSprites() const {
	return std::span<const SpriteDrawable>(&m_sprite, 1);
}
