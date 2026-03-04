#include "surface_manager.hpp"
#include "graphics_context.hpp"

SurfaceManager* SurfaceManager::s_instance = nullptr;

constexpr UINT WindowMessageSetRegion = WM_APP + 1;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_CLOSE: DestroyWindow(hWnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

static LRESULT CALLBACK ClickWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WindowMessageSetRegion:
		SetWindowRgn(hWnd, reinterpret_cast<HRGN>(wParam), false); // NOLINT
		SurfaceManager::getInstance().consumeClickableRegion();
		break;
	case WM_SETFOCUS: SurfaceManager::getInstance().getMainInput().notifyFocus(true); break;
	case WM_KILLFOCUS: SurfaceManager::getInstance().getMainInput().notifyFocus(false); break;
	case WM_LBUTTONDOWN: SurfaceManager::getInstance().getMainInput().notifyButtonPress(InputButton::MouseButtonLeft); break;
	case WM_LBUTTONUP: SurfaceManager::getInstance().getMainInput().notifyButtonRelease(InputButton::MouseButtonLeft); break;
	case WM_RBUTTONDOWN: SurfaceManager::getInstance().getMainInput().notifyButtonPress(InputButton::MouseButtonRight); break;
	case WM_RBUTTONUP: SurfaceManager::getInstance().getMainInput().notifyButtonRelease(InputButton::MouseButtonRight); break;
	case WM_MBUTTONDOWN: SurfaceManager::getInstance().getMainInput().notifyButtonPress(InputButton::MouseButtonMiddle); break;
	case WM_MBUTTONUP: SurfaceManager::getInstance().getMainInput().notifyButtonRelease(InputButton::MouseButtonMiddle); break;
	case WM_KEYDOWN: SurfaceManager::getInstance().getMainInput().notifyButtonPress(InputButton((lParam >> 16) & 0x1FF)); break;
	case WM_KEYUP: SurfaceManager::getInstance().getMainInput().notifyButtonRelease(InputButton((lParam >> 16) & 0x1FF)); break;
	case WM_CLOSE: DestroyWindow(hWnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

static void initializeWindowClasses(HINSTANCE hInstance) {
	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"Window";
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&windowClass);

	WNDCLASS clickWindowClass = {};
	clickWindowClass.style = 0;
	clickWindowClass.lpfnWndProc = ClickWndProc;
	clickWindowClass.hInstance = hInstance;
	clickWindowClass.lpszClassName = L"ClickWindow";
	clickWindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&clickWindowClass);
}

static BOOL CALLBACK createScreenSurface(HMONITOR hMonitor, HDC /* hdcMonitor */, LPRECT /* lprcMonitor */, LPARAM lParam) {
	auto hInstance = reinterpret_cast<HINSTANCE>(lParam); // NOLINT

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
	    int(x),
	    int(y),
	    int(w),
	    int(h),
	    nullptr,
	    nullptr,
	    hInstance,
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
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

	ComPtr<IDXGISwapChain1> swapchain;
	handleFatalError(
	    GraphicsContext::getInstance().getFactory()->CreateSwapChainForComposition(
	        GraphicsContext::getInstance().getDevice(), &swapchainDesc, nullptr, swapchain.GetAddressOf()
	    ),
	    "Could not create swapchain"
	);

	SurfaceManager::getInstance().m_screenSurfaces.emplace_back(
	    std::make_unique<ScreenSurface>(hWnd, std::move(swapchain), glm::uvec2(w, h), glm::ivec2(x, y))
	);
	ShowWindow(hWnd, SW_SHOWNOACTIVATE);

	return TRUE;
}

void SurfaceManager::initialize(HINSTANCE hInstance) {
	assert(!s_instance);
	s_instance = new SurfaceManager(hInstance);

	int vScreenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int vScreenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int vScreenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vScreenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	s_instance->m_clickWindow = CreateWindowEx(
	    WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
	    L"ClickWindow",
	    L"",
	    WS_POPUP,
	    vScreenX,
	    vScreenY,
	    vScreenW,
	    vScreenH,
	    nullptr,
	    nullptr,
	    hInstance,
	    nullptr
	);

	SetWindowRgn(s_instance->m_clickWindow, CreateRectRgn(0, 0, 0, 0), false);
	ShowWindow(s_instance->m_clickWindow, SW_SHOW);

	EnumDisplayMonitors(nullptr, nullptr, createScreenSurface, reinterpret_cast<LPARAM>(hInstance));
	GraphicsContext::getInstance().getCompositionDevice()->Commit();
}

void SurfaceManager::close() {
	assert(s_instance);
	delete s_instance;
	s_instance = nullptr;
}

SurfaceManager::SurfaceManager(HINSTANCE hInstance) : m_clickWindow(nullptr), m_rgn(nullptr), m_vScreenBounds{} {
	initializeWindowClasses(hInstance);

	int vScreenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int vScreenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int vScreenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vScreenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	m_vScreenBounds = BoundingBox{ .min = glm::vec2(vScreenX, vScreenY), .max = glm::vec2(vScreenX + vScreenW, vScreenY + vScreenH) };
}

SurfaceManager::~SurfaceManager() {
	CloseWindow(m_clickWindow);
}

SurfaceManager& SurfaceManager::getInstance() {
	assert(s_instance);
	return *s_instance;
}

Surface* SurfaceManager::getSurface(HWND window) {
	auto it = m_surfaces.find(window);
	if(it == m_surfaces.end()) return nullptr;
	return it->second;
}

void SurfaceManager::pushClickableRegion(const BoundingBox region) {
	m_rgn = CreateRectRgn(
	    int(region.min.x - m_vScreenBounds.min.x),
	    int(region.min.y - m_vScreenBounds.min.y),
	    int(region.max.x - m_vScreenBounds.min.x),
	    int(region.max.y - m_vScreenBounds.min.y)
	);
	PostMessage(m_clickWindow, WindowMessageSetRegion, reinterpret_cast<WPARAM>(m_rgn), 0);
}
