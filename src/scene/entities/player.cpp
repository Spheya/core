#include "player.hpp"
#include "rendering/sprite_atlas.hpp"

Player::Player() {
	Sprite runAnimSprites[] = {
		SpriteAtlas::getInstance().get("player_run_1.png"), SpriteAtlas::getInstance().get("player_run_2.png"),
		SpriteAtlas::getInstance().get("player_run_3.png"), SpriteAtlas::getInstance().get("player_run_4.png"),
		SpriteAtlas::getInstance().get("player_run_5.png"), SpriteAtlas::getInstance().get("player_run_6.png"),
	};

	m_animation = Animation(runAnimSprites, 12, 2, 0);
	localPhysicsBounds = { glm::vec2(-0.25f, 0.0f), glm::vec2(0.25f, 0.5f) };
	localClickBounds = { glm::vec2(-0.5f), glm::vec2(0.5f) };
}

void Player::onUpdate(const Time& time) {
	m_animation.update(time);
	m_sprite.sprite = m_animation.getCurrentFrame();
	m_sprite.matrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
}

std::span<const SpriteDrawable> Player::getSprites() const {
	return std::span<const SpriteDrawable>(&m_sprite, 1);
}
