#pragma once

#include "math.hpp"

struct BoundingBox {
	glm::vec2 min;
	glm::vec2 max;
};

struct IntBoundingBox {
	glm::ivec2 min;
	glm::ivec2 max;
};
