#pragma once

#include "math.hpp"
#include "time.hpp"

class Squisher {
public:
	Squisher() = default;
	Squisher(float amplitude, float frequency, float falloff, glm::vec2 origin = glm::vec2(0.0f)) :
	    amplitude(amplitude), frequency(frequency), falloff(falloff), origin(origin) {}

	void squish(const Time& time, float intensity = 1.0f) {
		m_start = time.time();
		m_intensity = intensity;
	}

	void squash(const Time& time, float intensity = 1.0f) {
		m_start = time.time();
		m_intensity = -intensity;
	}

	glm::mat4 calcMatrix(const Time& time) const {
		float t = float(time.time() - m_start);
		float jiggle = m_intensity * amplitude * std::sin(t * frequency * glm::two_pi<float>()) * std::exp(-falloff * t);
		glm::vec2 scale(1.0f / (1.0f + jiggle), 1.0f + jiggle);
		return glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(origin, 0.0f)), glm::vec3(scale, 1.0f)), glm::vec3(-origin, 0.0f));
	}

public:
	float amplitude = 1.0f;
	float frequency = 8.0f;
	float falloff = 20.0f;
	glm::vec2 origin = glm::vec2(0.0f);

private:
	float m_intensity = 0.0f;
	double m_start = 0.0f;
};
