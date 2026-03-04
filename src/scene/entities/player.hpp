#pragma once

#include "animation/character_animator.hpp"
#include "input/input.hpp"
#include "math.hpp"
#include "scene/entity.hpp"

class Player : public Entity {
public:
	Player(CharacterAnimations animations, const Input* input = nullptr);

	void setInput(const Input* input);

	virtual void onUpdate(const Time& time) override;

	virtual std::span<const SpriteDrawable> getSprites() const override;

private:
	void updateClickableRegion();
	void move(glm::vec2 delta);

private:
	const InputAxis1D* m_movementInput;
	const InputAction* m_jumpInput;
	const InputAction* m_duckInput;

	glm::vec2 m_velocity;
	bool m_grounded;

	bool m_flipped;
	SpriteDrawable m_sprite;
	CharacterAnimator m_animator;
};
