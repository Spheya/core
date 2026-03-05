#pragma once

#include "math.hpp"

class Sprite {
public:
	constexpr Sprite() = default;

	constexpr Sprite(unsigned x, unsigned y, unsigned width, unsigned height, unsigned atlasWidth, unsigned atlasHeight) :
	    m_width(width),
	    m_height(height),
	    m_scaleX(float(width) / float(atlasWidth)),
	    m_scaleY(float(height / float(atlasHeight))),
	    m_offsetX(float(x) / float(atlasWidth)),
	    m_offsetY(float(y) / float(atlasHeight)) {}

	[[nodiscard]] glm::uvec2 getDimensions() const { return glm::vec2(m_width, m_height); }
	[[nodiscard]] constexpr unsigned getWidth() const { return m_width; }
	[[nodiscard]] constexpr unsigned getHeight() const { return m_height; }
	[[nodiscard]] glm::vec4 getScaleOffset() const { return glm::vec4(m_scaleX, m_scaleY, m_offsetX, m_offsetY); }

private:
	unsigned m_width = 0;
	unsigned m_height = 0;

	float m_scaleX = 0.0f;
	float m_scaleY = 0.0f;
	float m_offsetX = 0.0f;
	float m_offsetY = 0.0f;
};
