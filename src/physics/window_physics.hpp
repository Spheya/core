#pragma once

#include <vector>

#include "bounding_box.hpp"
#include "intersection.hpp"
#include "math.hpp"

class WindowPhysics {
public:
	void update();
	void generateScreenBounds();

	[[nodiscard]] bool overlaps(const BoundingBox& box) const;
	[[nodiscard]] bool overlaps(glm::vec2 pos) const;
	[[nodiscard]] Intersection rayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance = 1e32f) const;
	[[nodiscard]] Intersection boxCast(BoundingBox origin, glm::vec2 direction, float maxDistance = 1e32f) const;

private:
	struct PhysicsWindow {
		BoundingBox bbox;
		bool ignore;
	};

private:
	std::vector<PhysicsWindow> m_hitboxes;
	std::vector<BoundingBox> m_screenEdges;
	std::vector<BoundingBox> m_taskBar;
};
