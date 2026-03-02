#pragma once

#include <unordered_map>

#include "math.hpp"
#include "platform.hpp"

class Surface {
public:
	[[nodiscard]] static Surface* get(HWND window);
	[[nodiscard]] static size_t count();

public:
	Surface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::uvec2 initialDimensions, glm::ivec2 position);
	Surface(const Surface&) = delete;
	Surface& operator=(const Surface&) = delete;
	Surface(Surface&& other) noexcept;
	Surface& operator=(Surface&& other) noexcept;
	virtual ~Surface();

	void resizeSwapchain(glm::uvec2 dimensions);

	[[nodiscard]] HWND getWindow() const { return m_window; }
	[[nodiscard]] IDXGISwapChain* getSwapchain() const { return m_swapchain.Get(); }
	[[nodiscard]] ID3D11RenderTargetView* getRenderTargetView() const { return m_rtv.Get(); }

	[[nodiscard]] glm::ivec2 getPosition() const { return m_position; }
	[[nodiscard]] glm::uvec2 getDimensions() const { return m_dimensions; }
	[[nodiscard]] unsigned getWidth() const { return m_dimensions.x; }
	[[nodiscard]] unsigned getHeight() const { return m_dimensions.y; }

private:
	void destroy();
	void loadRtv();

private:
	static std::unordered_map<HWND, Surface*> s_surfaces;

private:
	HWND m_window;
	ComPtr<IDXGISwapChain> m_swapchain;
	ComPtr<ID3D11RenderTargetView> m_rtv;
	glm::uvec2 m_dimensions;
	glm::ivec2 m_position;
};

class ScreenSurface : public Surface {
public:
	ScreenSurface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::uvec2 initialDimensions, glm::ivec2 position);

	[[nodiscard]] IDCompositionTarget* getTarget() const { return m_target.Get(); }
	[[nodiscard]] IDCompositionVisual* getVisual() const { return m_visual.Get(); }
	[[nodiscard]] glm::mat4 getProjectionMatrix() const {
		glm::vec2 p = getPosition();
		glm::vec2 s = getDimensions();

		// clang-format off
		return glm::mat4(
			+2.0f / s.x, 0.0f, 0.0f, -1.0f - 2.0f * p.x / s.x,
			0.0f, -2.0f / s.y, 0.0f, +1.0f + 2.0f * p.y / s.y,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		// clang-format on
	}

private:
	ComPtr<IDCompositionTarget> m_target;
	ComPtr<IDCompositionVisual> m_visual;
};
