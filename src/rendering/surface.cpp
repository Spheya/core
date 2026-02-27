#include "surface.hpp"
#include "graphics_context.hpp"

std::unordered_map<HWND, Surface*> Surface::s_surfaces;

Surface* Surface::get(HWND window) {
	auto it = s_surfaces.find(window);
	if(it == s_surfaces.end()) return nullptr;
	return it->second;
}

size_t Surface::count() {
	return s_surfaces.size();
}

Surface::Surface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::uvec2 initialDimensions, glm::ivec2 position) :
    m_window(window), m_swapchain(std::move(swapchain)), m_dimensions(initialDimensions), m_position(position) {
	loadRtv();
	s_surfaces.emplace(window, this);
}

Surface::Surface(Surface&& other) noexcept :
    m_window(other.m_window),
    m_swapchain(std::move(other.m_swapchain)),
    m_rtv(std::move(other.m_rtv)),
    m_dimensions(other.m_dimensions),
    m_position(other.m_position) {
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
	m_position = other.m_position;
	s_surfaces.at(other.m_window) = this;
	other.m_window = nullptr;
	return *this;
}

Surface::~Surface() {
	destroy();
}

void Surface::resizeSwapchain(glm::uvec2 dimensions) {
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

ScreenSurface::ScreenSurface(HWND window, ComPtr<IDXGISwapChain> swapchain, glm::uvec2 initialDimensions, glm::ivec2 position) :
    Surface(window, std::move(swapchain), initialDimensions, position) {
	auto* compDevice = GraphicsContext::getInstance().getCompositionDevice();
	handleFatalError(compDevice->CreateTargetForHwnd(window, true, m_target.GetAddressOf()), "Could not create a DirectComposition target");
	handleFatalError(compDevice->CreateVisual(m_visual.GetAddressOf()), "Could not create a DirectComposition visual");
	handleFatalError(m_visual->SetContent(getSwapchain()), "Could not assign swapchain to DirectComposition visual");
	handleFatalError(m_target->SetRoot(m_visual.Get()), "Could not assign DirectComposition visual to target");
}
