#include "graphics_context.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

#include "sprite_atlas.hpp"

static constexpr unsigned MaxInstances = 256;

GraphicsContext* GraphicsContext::s_instance = nullptr;

namespace {
	struct InstanceData {
		glm::mat4 matrix;
		glm::vec4 texCoordSt;
	};
} // namespace

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case WM_CLOSE: DestroyWindow(hWnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

static BOOL CALLBACK createScreenSurface(HMONITOR hMonitor, HDC /* hdcMonitor */, LPRECT /* lprcMonitor */, LPARAM lParam) {
	HINSTANCE hInstance = (HINSTANCE)lParam; // NOLINT

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

	GraphicsContext::getInstance().m_screenSurfaces.emplace_back(
	    std::make_unique<ScreenSurface>(hWnd, std::move(swapchain), glm::uvec2(w, h), glm::ivec2(x, y))
	);
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
	EnumDisplayMonitors(nullptr, nullptr, createScreenSurface, (LPARAM)hInstance); // NOLINT

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

	// Setup buffers
	D3D11_BUFFER_DESC cameraBufferDesc = {};
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(glm::mat4) * 2;
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	handleFatalError(m_device->CreateBuffer(&cameraBufferDesc, nullptr, m_cameraBuffer.GetAddressOf()), "Could not create camera buffer");

	D3D11_BUFFER_DESC instanceBufferDesc = {};
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceData) * MaxInstances;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	handleFatalError(m_device->CreateBuffer(&instanceBufferDesc, nullptr, m_instanceBuffer.GetAddressOf()), "Could not create instancing buffer");

	// Setup samplers
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	handleFatalError(m_device->CreateSamplerState(&samplerDesc, m_pointSampler.GetAddressOf()), "Could not create point sampler state");

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

void GraphicsContext::prepareCameraMatrices(const Camera& camera) {
	D3D11_MAPPED_SUBRESOURCE cameraBufferResource;
	handleFatalError(
	    m_context->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cameraBufferResource), "Could not map camera buffer to CPU memory"
	);
	memcpy(cameraBufferResource.pData, &camera, sizeof(glm::mat4) * 2);
	m_context->Unmap(m_cameraBuffer.Get(), 0);
}

void GraphicsContext::drawSprites(const Camera& camera, std::span<const SpriteDrawable> drawables) {
	assert(camera.target);

	// Prepare render target
	D3D11_VIEWPORT viewport = {};
	viewport.Width = float(camera.target->getWidth());
	viewport.Height = float(camera.target->getHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	auto* rtv = camera.target->getRenderTargetView();
	auto* srv = SpriteAtlas::getInstance().getShaderResourceView();

	m_context->OMSetRenderTargets(1, &rtv, nullptr);
	m_context->RSSetViewports(1, &viewport);
	m_context->ClearRenderTargetView(rtv, clearColor);
	prepareCameraMatrices(camera);

	// Prepare rendering state
	UINT strides[] = { sizeof(Vertex), sizeof(InstanceData) };
	UINT offsets[] = { 0, 0 };
	ID3D11Buffer* vertexBuffers[] = { m_quadMesh->m_vertexBuffer.Get(), m_instanceBuffer.Get() };

	m_context->VSSetShader(m_defaultVertexShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_defaultPixelShader.Get(), nullptr, 0);
	m_context->PSSetShaderResources(0, 1, &srv);
	m_context->PSSetSamplers(0, 1, m_pointSampler.GetAddressOf());
	m_context->VSSetConstantBuffers(0, 1, m_cameraBuffer.GetAddressOf());

	m_context->IASetInputLayout(m_defaultInputLayout.Get());
	m_context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	m_context->IASetIndexBuffer(m_quadMesh->m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw everything
	for(unsigned i = 0; i < unsigned(drawables.size()); i += MaxInstances) {
		unsigned batchSize = std::min(MaxInstances, unsigned(drawables.size()) - i);
		D3D11_MAPPED_SUBRESOURCE instanceBufferResource;
		handleFatalError(
		    m_context->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &instanceBufferResource),
		    "Could not map instance buffer to CPU memory"
		);
		auto* instanceData = static_cast<InstanceData*>(instanceBufferResource.pData);
		for(unsigned j = 0; j < batchSize; ++j)
			instanceData[j] = InstanceData{ .matrix = drawables[i + j].matrix, .texCoordSt = drawables[i + j].sprite.getScaleOffset() };
		m_context->Unmap(m_instanceBuffer.Get(), 0);
		m_context->DrawIndexedInstanced(m_quadMesh->getIndexCount(), batchSize, 0, 0, 0);
	}
}

void GraphicsContext::loadResources() {
	constexpr char defaultVertexSource[] = {
#embed "embed/default_vs.cso"
	};
	constexpr char defaultPixelSource[] = {
#embed "embed/default_ps.cso"
	};

	constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },

		{ "MODEL",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MODEL",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MODEL",    2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "MODEL",    3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "ST",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	m_device->CreateInputLayout(layout, sizeof(layout) / sizeof(*layout), defaultVertexSource, sizeof(defaultVertexSource), &m_defaultInputLayout);
	m_device->CreateVertexShader(defaultVertexSource, sizeof(defaultVertexSource), nullptr, &m_defaultVertexShader);
	m_device->CreatePixelShader(defaultPixelSource, sizeof(defaultPixelSource), nullptr, &m_defaultPixelShader);

	constexpr Vertex quadVertices[] = {
		{ .position = glm::vec3(-0.5f, +0.5f, 0.0f), .uv = glm::vec2(0.0f, 1.0f) },
		{ .position = glm::vec3(+0.5f, +0.5f, 0.0f), .uv = glm::vec2(1.0f, 1.0f) },
		{ .position = glm::vec3(+0.5f, -0.5f, 0.0f), .uv = glm::vec2(1.0f, 0.0f) },
		{ .position = glm::vec3(-0.5f, -0.5f, 0.0f), .uv = glm::vec2(0.0f, 0.0f) },
	};

	constexpr unsigned quadIndices[] = { 0, 2, 1, 0, 3, 2 };

	m_quadMesh = std::unique_ptr<Mesh>(new Mesh(m_device.Get(), quadVertices, quadIndices));
}
