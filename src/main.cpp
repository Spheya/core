#include <thread>

#include "platform.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"

static std::atomic_bool s_closeRequested; // NOLINT

static void applicationLoop() {
	while(!s_closeRequested) {
		for(const auto& surface : GraphicsContext::getInstance().getScreenSurfaces()) {
			float aspect = float(surface->getWidth()) / float(surface->getHeight());
			Camera camera = { .view = glm::mat4(1.0f), .proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f), .target = surface.get() };
			GraphicsContext::getInstance().draw(camera);
			surface->getSwapchain()->Present(1, 0);
		}
	}
}

static int runApp(HINSTANCE hInstance) {
	GraphicsContext::initialize(hInstance);
	SpriteAtlas::load();

	std::thread app(applicationLoop);

	MSG msg = {};
	while(GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	s_closeRequested = true;
	app.join();

	SpriteAtlas::destroy();
	GraphicsContext::close();
	return 0;
}

#ifdef _CONSOLE
int main() {
	return runApp(GetModuleHandle(nullptr));
}
#endif

#ifdef _WINDOWS
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /* hPrevInstance */, _In_ PSTR /* lpCmdLine */, _In_ int /* nShowCmd */) {
	return runApp(hInstance);
}
#endif
