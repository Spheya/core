#include "graphics_context.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

GraphicsContext* GraphicsContext::s_instance = nullptr;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_CLOSE: DestroyWindow(hWnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

static BOOL CALLBACK createScreenSurface(HMONITOR hMonitor, HDC /* hdcMonitor */, LPRECT /* lprcMonitor */, LPARAM lParam) {
	MONITORINFOEX mi = {};
	mi.cbSize = sizeof(MONITORINFOEX);

	if(!GetMonitorInfo(hMonitor, &mi)) fatalError("Could not obtain monitor info");

	unsigned x = mi.rcMonitor.left;
	unsigned y = mi.rcMonitor.top;
	unsigned w = mi.rcMonitor.right - mi.rcMonitor.left;
	unsigned h = mi.rcMonitor.bottom - mi.rcMonitor.top;

	HWND hWnd = CreateWindowEx(
	    WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
	    L"Window",
	    L"",
	    WS_POPUP,
	    x,
	    y,
	    w,
	    h,
	    nullptr,
	    nullptr,
	    reinterpret_cast<HINSTANCE>(lParam),
	    nullptr
	);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = w;
	swapchainDesc.Height = h;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = FALSE;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = 2;
	// swapchainDesc.Scaling = DXGI_SCALING_NONE;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

	ComPtr<IDXGISwapChain1> swapchain;
	handleFatalError(
	    GraphicsContext::getInstance().getFactory()->CreateSwapChainForComposition(
	        GraphicsContext::getInstance().getDevice(), &swapchainDesc, nullptr, swapchain.GetAddressOf()
	    ),
	    "Could not create swapchain"
	);

	GraphicsContext::getInstance().m_screenSurfaces.emplace_back(std::make_unique<ScreenSurface>(hWnd, std::move(swapchain), glm::ivec2(w, h)));
	ShowWindow(hWnd, SW_SHOW);

	return TRUE;
}

static void initializeWindowClasses(HINSTANCE hInstance) {
	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"Window";
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&windowClass);
}

void GraphicsContext::initialize(HINSTANCE hInstance) {
	assert(!s_instance);
	s_instance = new GraphicsContext();

	initializeWindowClasses(hInstance);
	EnumDisplayMonitors(nullptr, nullptr, createScreenSurface, reinterpret_cast<LPARAM>(hInstance));

	s_instance->loadResources();
	s_instance->getCompositionDevice()->Commit();
}

void GraphicsContext::close() {
	assert(s_instance);
	delete s_instance;
	s_instance = nullptr;
}

GraphicsContext& GraphicsContext::getInstance() {
	assert(s_instance);
	return *s_instance;
}

GraphicsContext::GraphicsContext() {
	// Setup DX11
	[[maybe_unused]] D3D_FEATURE_LEVEL featureLevel;

	handleFatalError(
	    D3D11CreateDevice(
	        nullptr,
	        D3D_DRIVER_TYPE_HARDWARE,
	        nullptr,
	        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
	        nullptr,
	        0,
	        D3D11_SDK_VERSION,
	        &m_device,
	        &featureLevel,
	        &m_context
	    ),
	    "Could not setup DX11 device"
	);

	// Setup DirectComposition
	handleFatalError(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(m_compDevice.GetAddressOf())), "Could not create a DirectComposition context");

	// Setup factory
	handleFatalError(CreateDXGIFactory(IID_PPV_ARGS(&m_factory)), "Could not create IDXGIFactory");

	// Print Device Info
#ifndef SHIPPING
	ComPtr<IDXGIDevice> dxgiDevice;
	m_device.As(&dxgiDevice);

	ComPtr<IDXGIAdapter> adapter;
	DXGI_ADAPTER_DESC desc;
	dxgiDevice->GetAdapter(&adapter);
	adapter->GetDesc(&desc);

	const char* featureLevelString;
	switch(featureLevel) {
	case D3D_FEATURE_LEVEL_1_0_GENERIC: featureLevelString = "1.0 Generic"; break;
	case D3D_FEATURE_LEVEL_1_0_CORE: featureLevelString = "1.0 Core"; break;
	case D3D_FEATURE_LEVEL_9_1: featureLevelString = "9.1"; break;
	case D3D_FEATURE_LEVEL_9_2: featureLevelString = "9.2"; break;
	case D3D_FEATURE_LEVEL_9_3: featureLevelString = "9.3"; break;
	case D3D_FEATURE_LEVEL_10_0: featureLevelString = "10.0"; break;
	case D3D_FEATURE_LEVEL_10_1: featureLevelString = "10.1"; break;
	case D3D_FEATURE_LEVEL_11_0: featureLevelString = "11.0"; break;
	case D3D_FEATURE_LEVEL_11_1: featureLevelString = "11.1"; break;
	case D3D_FEATURE_LEVEL_12_0: featureLevelString = "12.0"; break;
	case D3D_FEATURE_LEVEL_12_1: featureLevelString = "12.1"; break;
	case D3D_FEATURE_LEVEL_12_2: featureLevelString = "12.2"; break;
	}

	logger::log("DirectX version: {}", featureLevelString);

	size_t out;
	char str[256];
	wcstombs_s(&out, str, 256, desc.Description, _TRUNCATE);

	logger::log("DirectX Device: {}", str);
	logger::log("Device Id: {:#x}", desc.DeviceId);
	logger::log("Device Vendor Id: {:#x}", desc.VendorId);
	logger::log("Video Memory: {} MB\n", (desc.DedicatedVideoMemory / (1024ull * 1024ull)));
#endif
}

void GraphicsContext::draw(const Surface& surface) {
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	auto* rtv = surface.getRenderTargetView();

	D3D11_VIEWPORT viewport = {};
	viewport.Width = float(surface.getWidth());
	viewport.Height = float(surface.getHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ID3D11Buffer* buffer = m_quadMesh->m_vertexBuffer.Get();

	m_context->OMSetRenderTargets(1, &rtv, nullptr);
	m_context->RSSetViewports(1, &viewport);
	m_context->ClearRenderTargetView(rtv, clearColor);

	m_context->VSSetShader(m_defaultVertexShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_defaultPixelShader.Get(), nullptr, 0);
	m_context->IASetInputLayout(m_defaultInputLayout.Get());
	m_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	m_context->IASetIndexBuffer(m_quadMesh->m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->DrawIndexed(m_quadMesh->getIndexCount(), 0, 0);
}

void GraphicsContext::loadResources() {
	constexpr char defaultVertexSource[] = {
#embed "embed/default_vs.cso"
	};
	constexpr char defaultPixelSource[] = {
#embed "embed/default_ps.cso"
	};

	constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	m_device->CreateInputLayout(layout, 1, defaultVertexSource, sizeof(defaultVertexSource), &m_defaultInputLayout);
	m_device->CreateVertexShader(defaultVertexSource, sizeof(defaultVertexSource), nullptr, &m_defaultVertexShader);
	m_device->CreatePixelShader(defaultPixelSource, sizeof(defaultPixelSource), nullptr, &m_defaultPixelShader);

	constexpr Vertex quadVertices[] = {
		{ glm::vec3(-0.5f, +0.5f, 0.0f) },
		{ glm::vec3(+0.5f, +0.5f, 0.0f) },
		{ glm::vec3(+0.5f, -0.5f, 0.0f) },
		{ glm::vec3(-0.5f, -0.5f, 0.0f) },
	};

	constexpr unsigned quadIndices[] = { 0, 1, 2, 0, 2, 3 };

	m_quadMesh = std::unique_ptr<Mesh>(new Mesh(m_device.Get(), quadVertices, quadIndices));
}
