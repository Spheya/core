#pragma once

#include "rendering/animation.hpp"
#include "scene/entity.hpp"

class Player : public Entity {
public:
	Player();

	virtual void onUpdate(const Time& time) override;

	virtual std::span<const SpriteDrawable> getSprites() const override;

private:
	SpriteDrawable m_sprite;
	Animation m_animation;
};
