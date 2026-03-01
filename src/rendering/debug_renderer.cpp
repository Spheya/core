#include "debug_renderer.hpp"

#ifdef _DEBUG

	#include "graphics_context.hpp"

DebugRenderer::DebugRenderer() {
	constexpr char lineVertexSource[] = {
	#embed "embed/line_vs.cso"
	};
	constexpr char linePixelSource[] = {
	#embed "embed/line_ps.cso"
	};

	constexpr D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	auto* device = GraphicsContext::getInstance().getDevice();

	handleFatalError(
	    device->CreateInputLayout(layout, sizeof(layout) / sizeof(*layout), lineVertexSource, sizeof(lineVertexSource), &m_lineInputLayout),
	    "Can not load debug vertex layout"
	);
	handleFatalError(
	    device->CreateVertexShader(lineVertexSource, sizeof(lineVertexSource), nullptr, &m_lineVertexShader), "Can not load debug vertex shader"
	);
	handleFatalError(
	    device->CreatePixelShader(linePixelSource, sizeof(linePixelSource), nullptr, &m_linePixelShader), "Can not load debug pixel shader"
	);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(DebugRenderer::Vertex) * MaxVertices;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	handleFatalError(device->CreateBuffer(&bufferDesc, nullptr, &m_lineBuffer), "Could not allocate debug line buffer");
}

void DebugRenderer::line(glm::vec2 a, glm::vec2 b, glm::vec4 color) {
	m_vertices.emplace_back(glm::vec3(a.x, a.y, 0.0f), color);
	m_vertices.emplace_back(glm::vec3(b.x, b.y, 0.0f), color);
}

void DebugRenderer::box(const BoundingBox& box, glm::vec4 color) {
	line(glm::vec2(box.min.x, box.min.y), glm::vec2(box.min.x, box.max.y), color);
	line(glm::vec2(box.min.x, box.min.y), glm::vec2(box.max.x, box.min.y), color);
	line(glm::vec2(box.max.x, box.min.y), glm::vec2(box.max.x, box.max.y), color);
	line(glm::vec2(box.min.x, box.max.y), glm::vec2(box.max.x, box.max.y), color);
}

void DebugRenderer::circle(glm::vec2 center, float radius, glm::vec4 color) {
	for(int i = 0; i < 16; ++i) {
		float alpha = i * glm::two_pi<float>() / 16.0f;
		float beta = (i + 1) * glm::two_pi<float>() / 16.0f;
		line(glm::vec2(sin(alpha), cos(alpha)) * radius + center, glm::vec2(sin(beta), cos(beta)) * radius + center, color);
	}
}

void DebugRenderer::draw() {
	if(m_vertices.size() > MaxVertices)
		logger::warn("Attempting to draw {} vertices, the debug renderer can only handle up to {}", m_vertices.size(), MaxVertices);

	size_t vertexCount = std::min(m_vertices.size(), MaxVertices);
	UINT stride = sizeof(DebugRenderer::Vertex);
	UINT offset = 0;

	auto* context = GraphicsContext::getInstance().getDeviceContext();

	D3D11_MAPPED_SUBRESOURCE lineBufferResource;
	handleFatalError(context->Map(m_lineBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &lineBufferResource), "Could not map line buffer to CPU memory");
	memcpy(lineBufferResource.pData, m_vertices.data(), sizeof(DebugRenderer::Vertex) * vertexCount);
	context->Unmap(m_lineBuffer.Get(), 0);

	context->VSSetShader(m_lineVertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_linePixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(m_lineInputLayout.Get());
	context->IASetVertexBuffers(0, 1, m_lineBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->Draw(vertexCount, 0);

	m_vertices.clear();
}

#endif
