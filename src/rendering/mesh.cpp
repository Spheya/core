#include "mesh.hpp"

Mesh::Mesh(ID3D11Device* device, std::span<const Vertex> vertices, std::span<const unsigned> indices) :
    m_vertexCount(unsigned(vertices.size())), m_indexCount(unsigned(indices.size())) {
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertices.data();

	handleFatalError(device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_vertexBuffer.GetAddressOf()), "Could not allocate vertex buffer");

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(unsigned) * indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = indices.data();

	handleFatalError(device->CreateBuffer(&indexBufferDesc, &indexBufferData, m_indexBuffer.GetAddressOf()), "Could not allocate index buffer");
}
