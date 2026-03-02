#include "sprite_atlas.hpp"

#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "graphics_context.hpp"

using json = nlohmann::json;

std::unique_ptr<SpriteAtlas> SpriteAtlas::s_instance;

void SpriteAtlas::load() {
	assert(!s_instance);
	s_instance = std::make_unique<SpriteAtlas>();

	constexpr char jsonFile[] = {
#embed "embed/atlas.json"
	};

	constexpr char pngFile[] = {
#embed "embed/atlas.png"
	};

	// Load json data
	json data = json::parse(jsonFile);
	glm::uvec2 atlasSize(data.at("width"), data.at("height"));

	for(const auto& sprite : data.at("regions")) {
		std::string name = sprite.at("name");
		glm::uvec2 position(sprite.at("x"), sprite.at("y"));
		glm::uvec2 size(sprite.at("width"), sprite.at("height"));
		s_instance->m_sprites.emplace(name, Sprite(position, size, atlasSize));
	}

	// Load texture data
	int w;
	int h;
	int components;
	unsigned char* imageData = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(pngFile), sizeof(pngFile), &w, &h, &components, 4);
	assert(std::cmp_equal(w, atlasSize.x));
	assert(std::cmp_equal(h, atlasSize.y));

	// Create DX11 texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = atlasSize.x;
	textureDesc.Height = atlasSize.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData = {};
	textureData.pSysMem = imageData;
	textureData.SysMemPitch = atlasSize.x * 4;

	ID3D11Texture2D* texture;
	handleFatalError(GraphicsContext::getInstance().getDevice()->CreateTexture2D(&textureDesc, &textureData, &texture), "Could not create a texture");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	handleFatalError(
	    GraphicsContext::getInstance().getDevice()->CreateShaderResourceView(
	        texture, &shaderResourceViewDesc, s_instance->m_shaderResourceView.GetAddressOf()
	    ),
	    "Could not create a shader resource view"
	);
	texture->Release();

	// Free texture data
	stbi_image_free(imageData);
}

void SpriteAtlas::destroy() {
	s_instance.reset();
}
