#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

#include "platform.hpp"
#include "sprite.hpp"

// Todo: constexpr this, somehow
class SpriteAtlas {
public:
	static void load();
	static void destroy();

	[[nodiscard]] static SpriteAtlas& getInstance() {
		assert(s_instance);
		return *s_instance;
	}

	[[nodiscard]] const Sprite& get(const std::string& name) { return m_sprites.at(name); }
	[[nodiscard]] ID3D11ShaderResourceView* getShaderResourceView() const { return m_shaderResourceView.Get(); }

private:
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	std::unordered_map<std::string, Sprite> m_sprites;

	static std::unique_ptr<SpriteAtlas> s_instance;
};
