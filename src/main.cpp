#include <thread>

#define ZPACK_IMPLEMENTATION
#include <zpack.hpp>

#include "input/input_ids.hpp"
#include "physics/intersection.hpp"
#include "physics/window_physics.hpp"
#include "platform.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"
#include "rendering/surface_manager.hpp"
#include "scene/entities/player.hpp"
#include "scene/scene.hpp"
#include "time.hpp"

static std::atomic_bool s_closeRequested; // NOLINT

static zpack::FileLoader createFileLoader() {
	constexpr char assets[] = {
#embed "assets.pkg"
	};
	return zpack::FileLoader(assets, sizeof(assets));
}

static void applicationLoop() {
	constexpr Sprite playerSprites[] = {
		SpriteAtlas::get("player_duck.png"),   SpriteAtlas::get("player_fall.png"),  SpriteAtlas::get("player_idle_2.png"),
		SpriteAtlas::get("player_idle_1.png"), SpriteAtlas::get("player_jump.png"),  SpriteAtlas::get("player_run_1.png"),
		SpriteAtlas::get("player_run_2.png"),  SpriteAtlas::get("player_run_3.png"), SpriteAtlas::get("player_run_4.png"),
		SpriteAtlas::get("player_run_5.png"),  SpriteAtlas::get("player_run_6.png"), SpriteAtlas::get("player_slide.png"),
	};

	CharacterAnimations playerAnimations = {
		.idle = Animation(std::span(&playerSprites[2], 2), 24, 8),
		.run = Animation(std::span(&playerSprites[5], 6), 24, 2),
		.jump = Animation(std::span(&playerSprites[4], 1), 24, 2),
		.fall = Animation(std::span(&playerSprites[1], 1), 24, 2),
		.slide = Animation(std::span(&playerSprites[11], 1), 24, 2),
		.duck = Animation(std::span(&playerSprites[0], 1), 24, 2),
	};

	WindowPhysics windowPhysics;
	windowPhysics.generateScreenBounds();

	Scene scene;
	scene.addWindowPhysics(&windowPhysics);

	SurfaceManager::getInstance().getMainInput().add(
	    InputId_PlayerMovement, std::make_unique<InputAxis1D>(InputButton::KeyRight, InputButton::KeyLeft)
	);
	SurfaceManager::getInstance().getMainInput().add(InputId_PlayerJump, std::make_unique<InputAction>(InputButton::KeyUp));
	SurfaceManager::getInstance().getMainInput().add(InputId_PlayerDuck, std::make_unique<InputAction>(InputButton::KeyDown));

	auto* player = scene.addEntity(std::make_unique<Player>(std::move(playerAnimations), &SurfaceManager::getInstance().getMainInput()));
	player->position = glm::vec2(48.0f, 48.0f);
	player->flags = 1;

	Time time;

	while(!s_closeRequested) {
		time.update();
		SurfaceManager::getInstance().getMainInput().update();
		windowPhysics.update();

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
	zpack::FileLoader fileLoader = createFileLoader();

	GraphicsContext::initialize(fileLoader);
	SurfaceManager::initialize(hInstance);
	SpriteAtlas::load(fileLoader);

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
