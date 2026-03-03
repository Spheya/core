#include <thread>

#include "physics/intersection.hpp"
#include "physics/window_physics.hpp"
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
	WindowPhysics windowPhysics;
	Scene scene;
	scene.addWindowPhysics(&windowPhysics);

	auto* player = scene.addEntity(std::make_unique<Player>());
	player->position = glm::vec2(48.0f, 48.0f);
	player->flags = 1;

	SurfaceManager::getInstance().getMainInput().add(0, std::make_unique<InputAction>(InputButton::MouseButtonLeft));

	Time time;

	BoundingBox lineOrigin = { .min = glm::vec2(450.0f, 450.0f), .max = glm::vec2(500.0f, 475.0f) };

	while(!s_closeRequested) {
		time.update();
		SurfaceManager::getInstance().getMainInput().update();
		if(SurfaceManager::getInstance().getMainInput().getAction(0)->isPressed()) { logger::log("yippe"); }
		windowPhysics.update();

		scene.update(time);

#ifdef _DEBUG
		GraphicsContext::getInstance().getDebugRenderer().box(lineOrigin, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

		glm::vec2 bCenter = (lineOrigin.min + lineOrigin.max) * 0.5f;
		glm::vec2 rd = glm::normalize(SurfaceManager::getInstance().getMainInput().getMousePos() - bCenter);
		float maxDist = glm::distance(SurfaceManager::getInstance().getMainInput().getMousePos(), bCenter);
		Intersection hit = windowPhysics.boxCast(lineOrigin, rd, maxDist);
		glm::vec2 hitPos = bCenter + rd * hit.distance;

		BoundingBox tmp{
			.min = (lineOrigin.min - bCenter) + hitPos,
			.max = (lineOrigin.max - bCenter) + hitPos,
		};

		GraphicsContext::getInstance().getDebugRenderer().line(bCenter, hitPos);
		GraphicsContext::getInstance().getDebugRenderer().line(hitPos, hitPos + hit.normal * 32.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		GraphicsContext::getInstance().getDebugRenderer().box(tmp);

#endif

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
