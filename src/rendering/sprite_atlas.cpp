#include "sprite_atlas.hpp"

#include <memory>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "graphics_context.hpp"

ComPtr<ID3D11ShaderResourceView> SpriteAtlas::s_shaderResourceView;

void SpriteAtlas::load(const zpack::FileLoader& fileLoader) {
	std::vector<char> pngFile = fileLoader.get("atlas.png").read();

	// Load texture data
	int w = 0;
	int h = 0;
	int components = 0;
	unsigned char* imageData =
	    stbi_load_from_memory(reinterpret_cast<const unsigned char*>(pngFile.data()), pngFile.size(), &w, &h, &components, 4); // NOLINT

	// Create DX11 texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = UINT(w);
	textureDesc.Height = UINT(h);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData = {};
	textureData.pSysMem = imageData;
	textureData.SysMemPitch = UINT(w) * 4;

	ID3D11Texture2D* texture;
	handleFatalError(GraphicsContext::getInstance().getDevice()->CreateTexture2D(&textureDesc, &textureData, &texture), "Could not create a texture");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	handleFatalError(
	    GraphicsContext::getInstance().getDevice()->CreateShaderResourceView(texture, &shaderResourceViewDesc, s_shaderResourceView.GetAddressOf()),
	    "Could not create a shader resource view"
	);
	texture->Release();

	// Free texture data
	stbi_image_free(imageData);
}

void SpriteAtlas::destroy() {
	s_shaderResourceView.Reset();
}
