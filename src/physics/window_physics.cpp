#include "window_physics.hpp"

#include <algorithm>

#include "platform.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/surface_manager.hpp"

void WindowPhysics::update() {
	// todo: dont do this every frame
	m_hitboxes.clear();

	for(HWND hwnd = GetTopWindow(nullptr); hwnd; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
		if(!IsWindowVisible(hwnd)) continue;
		if(IsIconic(hwnd)) continue;
		if(GetWindow(hwnd, GW_OWNER) != nullptr) continue;
		if(GetWindowLongPtr(hwnd, GWL_EXSTYLE) & (WS_EX_TOOLWINDOW | WS_EX_TOPMOST)) continue;
		if(GetWindowLongPtr(hwnd, GWL_STYLE) & (WS_CHILD | WS_POPUP)) continue;

		RECT rect;
		if(GetWindowRect(hwnd, &rect)) m_hitboxes.emplace_back(glm::vec2(rect.left, rect.top), glm::vec2(rect.right, rect.bottom));
	}

#ifdef _DEBUG
	for(const auto& box : m_hitboxes) GraphicsContext::getInstance().getDebugRenderer().box(box, glm::vec4(0.0f, 1.0f, 0.0f, 0.3f));

	BoundingBox screen = SurfaceManager::getInstance().getVirtualScreenBounds();
	glm::vec2 s = screen.max - screen.min;

	for(BoundingBox box : m_screenEdges) {
		box.min -= glm::vec2(screen.min.x, screen.min.y);
		box.max -= glm::vec2(screen.min.x, screen.min.y);
		box.min = (box.min / s.x) * 300.0f;
		box.max = (box.max / s.x) * 300.0f;
		box.min += glm::vec2(32.0f);
		box.max += glm::vec2(32.0f);

		GraphicsContext::getInstance().getDebugRenderer().box(box, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	}
#endif
}

void WindowPhysics::generateScreenBounds() {
	m_screenEdges.clear();
	IntBoundingBox bounds = IntBoundingBox{ .min = glm::ivec2(SurfaceManager::getInstance().getVirtualScreenBounds().min),
		                                    .max = glm::ivec2(SurfaceManager::getInstance().getVirtualScreenBounds().max) };

	std::vector<IntBoundingBox> buffer;
	std::vector<IntBoundingBox> target;
	target.emplace_back(bounds.min - 100, bounds.max + 100);

	for(const auto& screen : SurfaceManager::getInstance().getScreenSurfaces()) {
		IntBoundingBox sbox = { .min = screen->getPosition(), .max = screen->getPosition() + glm::ivec2(screen->getDimensions()) };

		for(const auto& bbox : target) {
			if(!::overlaps(bbox, sbox)) {
				buffer.push_back(bbox);
				continue;
			}

			if(bbox.max.x > sbox.max.x) buffer.emplace_back(glm::ivec2(sbox.max.x, bbox.min.y), glm::ivec2(bbox.max.x, bbox.max.y));
			if(bbox.min.x < sbox.min.x) buffer.emplace_back(glm::ivec2(bbox.min.x, bbox.min.y), glm::ivec2(sbox.min.x, bbox.max.y));
			if(bbox.max.y > sbox.max.y) buffer.emplace_back(glm::ivec2(sbox.min.x, sbox.max.y), glm::ivec2(sbox.max.x, bbox.max.y));
			if(bbox.min.y < sbox.min.y) buffer.emplace_back(glm::ivec2(sbox.min.x, bbox.min.y), glm::ivec2(sbox.max.x, sbox.min.y));
		}

		std::swap(buffer, target);
		buffer.clear();
	}

	for(const auto& bbox : target) m_screenEdges.emplace_back(glm::vec2(bbox.min), glm::vec2(bbox.max));
}

bool WindowPhysics::overlaps(const BoundingBox& box) const {
	return std::ranges::any_of(m_hitboxes, [&box](const BoundingBox& hitbox) { return ::overlaps(hitbox, box); });
}

bool WindowPhysics::overlaps(glm::vec2 pos) const {
	return std::ranges::any_of(m_hitboxes, [pos](const BoundingBox& hitbox) { return ::overlaps(hitbox, pos); });
}

Intersection WindowPhysics::rayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance) const {
	Intersection miss{ .distance = maxDistance, .normal = glm::vec2(0.0f) };
	Intersection hit{ .distance = maxDistance, .normal = glm::vec2(0.0f) };
	float d = 0.0f;

	for(const auto& edge : m_screenEdges) {
		Intersection edgeHit = ::rayCast(origin, direction, edge, hit.distance, RayCastExclude::Exit);
		if(edgeHit.distance != hit.distance) hit = edgeHit;
	}

	for(const auto& hitBox : m_hitboxes) {
		if(::overlaps(hitBox, origin)) {
			Intersection exit = ::rayCast(origin, direction, hitBox, hit.distance, RayCastExclude::Entrance);
			if(exit.distance == hit.distance) break;

			origin += direction * exit.distance;
			d += exit.distance;
			hit.distance -= exit.distance;
		} else {
			auto newHit = ::rayCast(origin, direction, hitBox, hit.distance, RayCastExclude::Exit);
			if(newHit.distance != hit.distance) hit = newHit;
		}
	}

	if(hit.normal != glm::vec2(0.0f)) {
		hit.distance += d;
		return hit;
	}
	return miss;
}

Intersection WindowPhysics::boxCast(BoundingBox origin, glm::vec2 direction, float maxDistance) const {
	Intersection miss{ .distance = maxDistance, .normal = glm::vec2(0.0f) };
	Intersection hit{ .distance = maxDistance, .normal = glm::vec2(0.0f) };
	float d = 0.0f;

	for(const auto& edge : m_screenEdges) {
		Intersection edgeHit = ::boxCast(origin, direction, edge, hit.distance, RayCastExclude::Exit);
		if(edgeHit.distance != hit.distance) hit = edgeHit;
	}

	for(const auto& hitBox : m_hitboxes) {
		if(::overlaps(hitBox, origin)) {
			Intersection exit = ::boxCast(origin, direction, hitBox, hit.distance, RayCastExclude::Entrance);
			if(exit.distance == hit.distance) break;

			origin.min += direction * exit.distance;
			origin.max += direction * exit.distance;
			d += exit.distance;
			hit.distance -= exit.distance;
		} else {
			auto newHit = ::boxCast(origin, direction, hitBox, hit.distance, RayCastExclude::Exit);
			if(newHit.distance != hit.distance) hit = newHit;
		}
	}

	if(hit.normal != glm::vec2(0.0f)) {
		hit.distance += d;
		return hit;
	}
	return miss;
}
