#include <thread>

#include "platform.hpp"
#include "rendering/animation.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"
#include "rendering/surface_manager.hpp"
#include "scene/entities/player.hpp"
#include "scene/scene.hpp"
#include "time.hpp"

static std::atomic_bool s_closeRequested; // NOLINT

static void applicationLoop() {
	Scene scene;
	auto* player = scene.addEntity(std::make_unique<Player>());
	player->position = glm::vec2(48.0f, 48.0f);

	Time time;

	while(!s_closeRequested) {
		time.update();
		scene.update(time);

		for(const auto& surface : SurfaceManager::getInstance().getScreenSurfaces()) {
			Camera camera = { .view = glm::mat4(1.0f), .proj = surface->getProjectionMatrix(), .target = surface.get() };
			GraphicsContext::getInstance().drawSprites(camera, scene.buildSprites());

#ifdef _DEBUG
			GraphicsContext::getInstance().getDebugRenderer().draw();
#endif
		}

#ifdef _DEBUG
		GraphicsContext::getInstance().getDebugRenderer().clear();
#endif

		bool first = true;
		for(const auto& surface : SurfaceManager::getInstance().getScreenSurfaces()) {
			surface->getSwapchain()->Present(first ? 1 : 0, 0);
			first = false;
		}
	}
}

static int runApp(HINSTANCE hInstance) {
	GraphicsContext::initialize();
	SurfaceManager::initialize(hInstance);
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
