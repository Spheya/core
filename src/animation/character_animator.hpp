#pragma once

#include "animation.hpp"
#include "time.hpp"

enum class CharacterAnimation { Idle, Run, Jump, Fall, Slide, Duck };

struct CharacterAnimations {
	Animation idle;
	Animation run;
	Animation jump;
	Animation fall;
	Animation slide;
	Animation duck;
};

class CharacterAnimator {
public:
	CharacterAnimator(CharacterAnimations animations);

	bool setAnimation(CharacterAnimation animation);
	void update(const Time& time);
	void reset();
	[[nodiscard]] const Sprite& getCurrentFrame() const { return m_currentAnimation->getCurrentFrame(); }
	[[nodiscard]] const Animation& getCurrentAnimation() const { return *m_currentAnimation; }

private:
	std::unique_ptr<CharacterAnimations> m_animations;
	Animation* m_currentAnimation;
	bool m_shouldReset;
};
