#include "intersection.hpp"

Intersection rayCast(glm::vec2 origin, glm::vec2 direction, const BoundingBox& box, float maxDistance, RayCastExclude exclude) {
	Intersection result{ .distance = maxDistance, .normal = glm::vec2(0.0f) };

	glm::vec2 t1 = (box.min - origin) / direction;
	glm::vec2 t2 = (box.max - origin) / direction;

	glm::vec2 tmin = glm::min(t1, t2);
	glm::vec2 tmax = glm::max(t1, t2);

	float tnear = std::max(tmin.x, tmin.y);
	float tfar = std::min(tmax.x, tmax.y);

	if(tfar < tnear || tfar < 0.0f) return result;

	if(exclude == RayCastExclude::Entrance || tnear < 0.0f) {
		// Exiting
		if(exclude == RayCastExclude::Exit || tfar > maxDistance) return result;
		result.distance = tfar;

		if(tmax.x < tmax.y)
			result.normal = (t1.x > t2.x) ? glm::vec2(-1.0f, 0.0f) : glm::vec2(1.0f, 0.0f);
		else
			result.normal = (t1.y > t2.y) ? glm::vec2(0.0f, -1.0f) : glm::vec2(0.0f, 1.0f);
	} else {
		// Entering
		if(tnear > maxDistance) return result;
		result.distance = tnear;

		if(tmin.x > tmin.y)
			result.normal = (t1.x < t2.x) ? glm::vec2(-1.0f, 0.0f) : glm::vec2(+1.0f, 0.0f);
		else
			result.normal = (t1.y < t2.y) ? glm::vec2(0.0f, -1.0f) : glm::vec2(0.0f, +1.0f);
	}

	return result;
}

Intersection boxCast(const BoundingBox& origin, glm::vec2 direction, const BoundingBox& box, float maxDistance, RayCastExclude exclude) {
	glm::vec2 halfSize = (origin.max - origin.min) * 0.5f;
	glm::vec2 center = origin.min + halfSize;

	Intersection result{ .distance = maxDistance, .normal = glm::vec2(0.0f) };

	bool overlap = overlaps(box, center);
	if(overlap && exclude == RayCastExclude::Exit) return result;

	if(exclude == RayCastExclude::Entrance || overlap) {
		// Exit
		glm::vec2 t1 = (box.min + halfSize - center) / direction;
		glm::vec2 t2 = (box.max - halfSize - center) / direction;

		glm::vec2 tmin = glm::min(t1, t2);
		glm::vec2 tmax = glm::max(t1, t2);

		float tnear = std::max(tmin.x, tmin.y);
		float tfar = std::min(tmax.x, tmax.y);

		if(tfar < tnear || tfar < 0.0f || tfar > maxDistance) return result;
		result.distance = tfar;

		if(tmax.x < tmax.y)
			result.normal = (t1.x > t2.x) ? glm::vec2(-1.0f, 0.0f) : glm::vec2(1.0f, 0.0f);
		else
			result.normal = (t1.y > t2.y) ? glm::vec2(0.0f, -1.0f) : glm::vec2(0.0f, 1.0f);
	} else {
		// Entrance
		glm::vec2 t1 = (box.min - halfSize - center) / direction;
		glm::vec2 t2 = (box.max + halfSize - center) / direction;

		glm::vec2 tmin = glm::min(t1, t2);
		glm::vec2 tmax = glm::max(t1, t2);

		float tnear = std::max(tmin.x, tmin.y);
		float tfar = std::min(tmax.x, tmax.y);

		if(tfar < tnear || tfar < 0.0f || tnear > maxDistance) return result;

		result.distance = tnear;

		if(tmin.x > tmin.y)
			result.normal = (t1.x < t2.x) ? glm::vec2(-1.0f, 0.0f) : glm::vec2(+1.0f, 0.0f);
		else
			result.normal = (t1.y < t2.y) ? glm::vec2(0.0f, -1.0f) : glm::vec2(0.0f, +1.0f);
	}

	return result;
}
