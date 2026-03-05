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

	[[nodiscard]] static ID3D11ShaderResourceView* getShaderResourceView() { return s_shaderResourceView.Get(); }
	[[nodiscard]] static consteval Sprite get(const char* name);

private:
	static consteval std::string_view nextValue(std::string_view data) {
		size_t start = data.find_first_of(':') + 1;
		start = data.find_first_not_of(" \n\r\t", start);
		size_t end = data.find_first_not_of("1234567890", start);
		return data.substr(start, end - start);
	}

	static consteval unsigned readUnsigned(std::string_view str) {
		unsigned value = 0;
		for(char c : str) value = value * 10 + (c - '0');
		return value;
	}

private:
	static ComPtr<ID3D11ShaderResourceView> s_shaderResourceView;
};

consteval Sprite SpriteAtlas::get(const char* name) {
	constexpr static char jsonFile[] = {
#embed "embed/atlas.json"
	};
	constexpr static const char* xId = "\"x\"";
	constexpr static const char* yId = "\"y\"";
	constexpr static const char* widthId = "\"width\"";
	constexpr static const char* heightId = "\"height\"";
	constexpr static const std::string_view file(jsonFile, sizeof(jsonFile));

	unsigned atlasWidth = readUnsigned(nextValue(file.substr(file.find(widthId))));
	unsigned atlasHeight = readUnsigned(nextValue(file.substr(file.find(heightId))));

	std::string_view mySegment = file.substr(file.find(name));
	unsigned spriteX = readUnsigned(nextValue(mySegment.substr(mySegment.find(xId))));
	unsigned spriteY = readUnsigned(nextValue(mySegment.substr(mySegment.find(yId))));
	unsigned spriteWidth = readUnsigned(nextValue(mySegment.substr(mySegment.find(widthId))));
	unsigned spriteHeight = readUnsigned(nextValue(mySegment.substr(mySegment.find(heightId))));

	return Sprite(spriteX, spriteY, spriteWidth, spriteHeight, atlasWidth, atlasHeight);
}
