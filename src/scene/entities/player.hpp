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
	const Input* m_input;
	const InputAxis1D* m_movementInput;
	const InputAction* m_jumpInput;
	const InputAction* m_duckInput;

	glm::vec2 m_velocity;
	float m_slideCooldown;

	float m_slideBuffer;
	float m_duckJumpBuffer;
	float m_jumpBuffer;
	float m_coyoteTime;

	bool m_flipped;
	SpriteDrawable m_sprite;
	SpriteDrawable m_clickSprite;

	CharacterAnimator m_animator;
	Animation m_clickAnimation;
};
