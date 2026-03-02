#pragma once

#include <atomic>
#include <memory>
#include <span>
#include <vector>

#include "physics/bounding_box.hpp"
#include "surface.hpp"

class SurfaceManager {
	friend BOOL CALLBACK createScreenSurface(HMONITOR, HDC, LPRECT, LPARAM);
	friend class Surface;

public:
	static void initialize(HINSTANCE hInstance);
	static void close();

private:
	SurfaceManager(HINSTANCE hInstance);

public:
	SurfaceManager(SurfaceManager&) = delete;
	SurfaceManager& operator=(SurfaceManager&) = delete;
	SurfaceManager(SurfaceManager&&) = delete;
	SurfaceManager& operator=(SurfaceManager&&) = delete;
	~SurfaceManager();

public:
	[[nodiscard]] static SurfaceManager& getInstance();

	[[nodiscard]] Surface* getSurface(HWND window);
	[[nodiscard]] size_t getSurfaceCount() const { return m_surfaces.size(); }
	[[nodiscard]] std::span<const std::unique_ptr<ScreenSurface>> getScreenSurfaces() const { return m_screenSurfaces; }

	void pushClickableRegion(const BoundingBox region);
	bool canPushClickableRegion() const { return m_canPushRegion; }
	void consumeClickableRegion() { m_canPushRegion = true; }

	BoundingBox getVirtualScreenBounds() { return m_vScreenBounds; }

private:
	static SurfaceManager* s_instance;

private:
	std::unordered_map<HWND, Surface*> m_surfaces;
	std::vector<std::unique_ptr<ScreenSurface>> m_screenSurfaces;
	HWND m_clickWindow;

	BoundingBox m_vScreenBounds;

	HRGN m_rgn;
	std::atomic_bool m_canPushRegion = true;
	std::vector<char> m_rgnData;
};
