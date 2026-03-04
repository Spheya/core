#pragma once

#include <span>
#include <vector>

#include "rendering/sprite.hpp"
#include "time.hpp"

class Animation {
public:
	Animation() = default;

	Animation(std::span<const Sprite> sprites, unsigned frameRate, unsigned animateOn, unsigned frameAnimationOffset = 0) :
	    frameRate(frameRate),
	    animateOn(animateOn),
	    frameAnimationOffset(frameAnimationOffset),
	    m_sprites(sprites.begin(), sprites.end()),
	    m_currentSprite(m_sprites.begin()),
	    m_animationOffset(0ull) {}

	void reset(const Time& time) {
		m_animationOffset = calcFrame(time);
		m_currentSprite = m_sprites.begin();
	}

	void update(const Time& time) {
		unsigned long long frame = calcFrame(time) - m_animationOffset;
		m_currentSprite = m_sprites.begin() + unsigned(frame % m_sprites.size());
	}

	[[nodiscard]] const Sprite& getCurrentFrame() const { return *m_currentSprite; }

public:
	unsigned frameRate = 0;
	unsigned animateOn = 0;
	unsigned frameAnimationOffset = 0;

private:
	unsigned long long calcFrame(const Time& time) const {
		return ((unsigned long long)(time.time() * float(frameRate)) + frameAnimationOffset) / animateOn;
	}

private:
	std::vector<Sprite> m_sprites;
	std::vector<Sprite>::const_iterator m_currentSprite;
	unsigned long long m_animationOffset = 0;
};
