#include <thread>

#include "platform.hpp"
#include "rendering/animation.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"
#include "scene/entities/player.hpp"
#include "scene/scene.hpp"
#include "time.hpp"

static std::atomic_bool s_closeRequested; // NOLINT

static void applicationLoop() {
	Scene scene;
	scene.addEntity(std::make_unique<Player>());

	Time time;

	while(!s_closeRequested) {
		time.update();
		scene.update(time);

		for(const auto& surface : GraphicsContext::getInstance().getScreenSurfaces()) {
			float aspect = float(surface->getWidth()) / float(surface->getHeight());
			Camera camera = { .view = glm::mat4(1.0f), .proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f), .target = surface.get() };
			GraphicsContext::getInstance().drawSprites(camera, scene.buildSprites());

#ifdef _DEBUG
			GraphicsContext::getInstance().getDebugRenderer().draw();
#endif
		}

		bool first = true;
		for(const auto& surface : GraphicsContext::getInstance().getScreenSurfaces()) {
			surface->getSwapchain()->Present(first ? 1 : 0, 0);
			first = false;
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
