#pragma once

#include <memory>
#include <span>

#include "camera.hpp"
#include "debug_renderer.hpp"
#include "mesh.hpp"
#include "sprite_drawable.hpp"
#include "surface.hpp"

class GraphicsContext {
	friend BOOL CALLBACK createScreenSurface(HMONITOR, HDC, LPRECT, LPARAM);

public:
	static void initialize(HINSTANCE hInstance);
	static void close();
	static GraphicsContext& getInstance();

private:
	GraphicsContext();

public:
	[[nodiscard]] ID3D11Device* getDevice() const { return m_device.Get(); }
	[[nodiscard]] ID3D11DeviceContext* getDeviceContext() const { return m_context.Get(); }
	[[nodiscard]] IDCompositionDevice* getCompositionDevice() const { return m_compDevice.Get(); }
	[[nodiscard]] IDXGIFactory4* getFactory() const { return m_factory.Get(); }

	[[nodiscard]] std::span<const std::unique_ptr<ScreenSurface>> getScreenSurfaces() const { return m_screenSurfaces; }

	void prepareCameraMatrices(const Camera& camera);
	void drawSprites(const Camera& camera, std::span<const SpriteDrawable> drawables);

#ifdef _DEBUG
	[[nodiscard]] DebugRenderer& getDebugRenderer() const { return *m_debugRenderer; }
#endif

private:
	void loadResources();

private:
	static GraphicsContext* s_instance;

private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDCompositionDevice> m_compDevice;
	ComPtr<IDXGIFactory4> m_factory;

	ComPtr<ID3D11Buffer> m_cameraBuffer;
	ComPtr<ID3D11Buffer> m_instanceBuffer;

	ComPtr<ID3D11InputLayout> m_defaultInputLayout;
	ComPtr<ID3D11VertexShader> m_defaultVertexShader;
	ComPtr<ID3D11PixelShader> m_defaultPixelShader;

	ComPtr<ID3D11SamplerState> m_pointSampler;

	std::unique_ptr<Mesh> m_quadMesh;
	std::vector<std::unique_ptr<ScreenSurface>> m_screenSurfaces;

#ifdef _DEBUG
	std::unique_ptr<DebugRenderer> m_debugRenderer;
#endif
};
