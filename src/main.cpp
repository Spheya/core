#include <thread>

#include "platform.hpp"
#include "rendering/animation.hpp"
#include "rendering/graphics_context.hpp"
#include "rendering/sprite_atlas.hpp"
#include "time.hpp"

static std::atomic_bool s_closeRequested; // NOLINT

static void applicationLoop() {
	SpriteDrawable drawables[] = {
		{ .sprite = SpriteAtlas::getInstance().get("miku.png"), .matrix = glm::mat4(1.0f) },
		{ .sprite = SpriteAtlas::getInstance().get("player_idle_1.png"), .matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) }
	};

	Sprite runAnimSprites[] = {
		SpriteAtlas::getInstance().get("player_run_1.png"), SpriteAtlas::getInstance().get("player_run_2.png"),
		SpriteAtlas::getInstance().get("player_run_3.png"), SpriteAtlas::getInstance().get("player_run_4.png"),
		SpriteAtlas::getInstance().get("player_run_5.png"), SpriteAtlas::getInstance().get("player_run_6.png"),
	};

	Animation anim1(runAnimSprites, 24, 2, 0);
	Animation anim2(runAnimSprites, 24, 2, 1);

	Time time;

	while(!s_closeRequested) {
		time.update();

		anim1.update(time);
		anim2.update(time);
		drawables[0].sprite = anim1.getCurrentFrame();
		drawables[1].sprite = anim2.getCurrentFrame();

		glm::mat4 viewMat(1.0f);

		for(const auto& surface : GraphicsContext::getInstance().getScreenSurfaces()) {
			float aspect = float(surface->getWidth()) / float(surface->getHeight());
			Camera camera = { .view = viewMat, .proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f), .target = surface.get() };
			viewMat = glm::rotate(viewMat, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			GraphicsContext::getInstance().drawSprites(camera, drawables);

#ifdef _DEBUG
			GraphicsContext::getInstance().getDebugRenderer().line(glm::vec2(0.0f), glm::vec2(1.0f));
			GraphicsContext::getInstance().getDebugRenderer().box(BoundingBox(glm::vec2(-0.5f), glm::vec2(0.5f)), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			GraphicsContext::getInstance().getDebugRenderer().circle(glm::vec2(-1.2f, 0.0f), 0.1f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
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
