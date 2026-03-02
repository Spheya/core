#pragma once

#include <memory>
#include <span>
#include <vector>

#include "surface.hpp"

class SurfaceManager {
	friend BOOL CALLBACK createScreenSurface(HMONITOR, HDC, LPRECT, LPARAM);
	friend class Surface;

public:
	static void initialize(HINSTANCE hInstance);
	static void close();
	[[nodiscard]] static SurfaceManager& getInstance();

	[[nodiscard]] Surface* getSurface(HWND window);
	[[nodiscard]] size_t getSurfaceCount() const { return m_surfaces.size(); }
	[[nodiscard]] std::span<const std::unique_ptr<ScreenSurface>> getScreenSurfaces() const { return m_screenSurfaces; }

private:
	static SurfaceManager* s_instance;

private:
	std::unordered_map<HWND, Surface*> m_surfaces;
	std::vector<std::unique_ptr<ScreenSurface>> m_screenSurfaces;
};
