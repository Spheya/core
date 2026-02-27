#include "surface.hpp"

#include <unordered_map>

#include "graphics_context.hpp"

static std::unordered_map<HWND, Surface*> s_surfaces; // NOLINT

Surface::Surface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::ivec2 initialDimensions) :
    m_window(window), m_swapchain(std::move(swapchain)), m_dimensions(initialDimensions) {
	loadRtv();
	s_surfaces.emplace(window, this);
}

Surface::Surface(Surface&& other) noexcept :
    m_window(other.m_window), m_swapchain(std::move(other.m_swapchain)), m_rtv(std::move(other.m_rtv)), m_dimensions(other.m_dimensions) {
	s_surfaces.at(other.m_window) = this;
	other.m_window = nullptr;
	other.m_swapchain = nullptr;
	other.m_rtv = nullptr;
}

Surface& Surface::operator=(Surface&& other) noexcept {
	destroy();
	m_window = other.m_window;
	m_swapchain = std::move(other.m_swapchain);
	m_rtv = std::move(other.m_rtv);
	m_dimensions = other.m_dimensions;
	s_surfaces.at(other.m_window) = this;
	other.m_window = nullptr;
	return *this;
}

Surface::~Surface() {
	destroy();
}

Surface* Surface::get(HWND window) {
	auto it = s_surfaces.find(window);
	if(it == s_surfaces.end()) return nullptr;
	return it->second;
}

size_t Surface::count() {
	return s_surfaces.size();
}

void Surface::updateDimensions(glm::uvec2 dimensions) {
	m_dimensions = glm::max(dimensions, m_dimensions);

	auto* context = GraphicsContext::getInstance().getDeviceContext();
	context->OMSetRenderTargets(0, nullptr, nullptr);
	m_rtv.Reset();
	handleFatalError(m_swapchain->ResizeBuffers(0, dimensions.x, dimensions.y, DXGI_FORMAT_UNKNOWN, 0), "Error resizing swapchain");
	loadRtv();

	m_dimensions = dimensions;
}

void Surface::destroy() {
	s_surfaces.erase(m_window);
	m_rtv.Reset();
	m_swapchain.Reset();
	if(m_window) DestroyWindow(m_window);
}

void Surface::loadRtv() {
	void* backBuffer = nullptr;
	handleFatalError(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer), "Failed to obtain backbuffer of swapchain");
	handleFatalError(
	    GraphicsContext::getInstance().getDevice()->CreateRenderTargetView(static_cast<ID3D11Texture2D*>(backBuffer), nullptr, &m_rtv),
	    "Could not create a render target view"
	);
	static_cast<ID3D11Texture2D*>(backBuffer)->Release();
}

ScreenSurface::ScreenSurface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::ivec2 initialDimensions) :
    Surface(window, std::move(swapchain), initialDimensions) {
	auto* compDevice = GraphicsContext::getInstance().getCompositionDevice();
	handleFatalError(compDevice->CreateTargetForHwnd(window, true, m_target.GetAddressOf()), "Could not create a DirectComposition target");
	handleFatalError(compDevice->CreateVisual(m_visual.GetAddressOf()), "Could not create a DirectComposition visual");
	handleFatalError(m_visual->SetContent(getSwapchain()), "Could not assign swapchain to DirectComposition visual");
	handleFatalError(m_target->SetRoot(m_visual.Get()), "Could not assign DirectComposition visual to target");
}
