#pragma once

#include "bounding_box.hpp"
#include "math.hpp"

enum class RayCastExclude { None, Entrance, Exit };

struct Intersection {
	float distance;
	glm::vec2 normal;
};

inline Intersection pickClosestIntersection(Intersection a, Intersection b) {
	if(a.distance < b.distance) return a;
	return b;
}

inline bool overlaps(const IntBoundingBox& a, const IntBoundingBox& b) {
	return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

inline bool overlaps(const BoundingBox& a, const BoundingBox& b) {
	return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

inline bool overlaps(const BoundingBox& a, glm::vec2 b) {
	return a.max.x > b.x && a.min.x < b.x && a.max.y > b.y && a.min.y < b.y;
}

Intersection rayCast(
    glm::vec2 origin, glm::vec2 direction, const BoundingBox& box, float maxDistance = 1e32f, RayCastExclude exclude = RayCastExclude::None
);
Intersection boxCast(
    const BoundingBox& origin, glm::vec2 direction, const BoundingBox& box, float maxDistance = 1e32f, RayCastExclude exclude = RayCastExclude::None
);
