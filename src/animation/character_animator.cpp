#include "character_animator.hpp"

CharacterAnimator::CharacterAnimator(CharacterAnimations animations) :
    m_animations(std::make_unique<CharacterAnimations>(std::move(animations))), m_currentAnimation(&m_animations->idle), m_shouldReset(false) {}

void CharacterAnimator::setAnimation(CharacterAnimation animation) {
	Animation* selectedAnimation = (&m_animations->idle) + int(animation);
	if(m_currentAnimation != selectedAnimation) {
		m_currentAnimation = selectedAnimation;
		m_shouldReset = true;
	}
}

void CharacterAnimator::update(const Time& time) {
	if(m_shouldReset) {
		m_currentAnimation->reset(time);
		m_shouldReset = false;
	} else {
		m_currentAnimation->update(time);
	}
}

void CharacterAnimator::reset() {
	m_shouldReset = true;
}
