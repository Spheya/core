#pragma once

#ifdef _DEBUG

	#include <vector>

	#include "math.hpp"
	#include "physics/bounding_box.hpp"
	#include "platform.hpp"

class DebugRenderer {
public:
	constexpr static size_t MaxLines = 1024;
	constexpr static size_t MaxVertices = MaxLines * 2;

public:
	DebugRenderer();

	void line(glm::vec2 a, glm::vec2 b, glm::vec4 color = glm::vec4(1.0f));
	void box(const BoundingBox& box, glm::vec4 color = glm::vec4(1.0f));
	void circle(glm::vec2 center, float radius, glm::vec4 color = glm::vec4(1.0f));

	void draw();

private:
	struct Vertex {
		glm::vec3 pos;
		glm::vec4 color;
	};

private:
	std::vector<Vertex> m_vertices;

	ComPtr<ID3D11Buffer> m_lineBuffer;
	ComPtr<ID3D11InputLayout> m_lineInputLayout;
	ComPtr<ID3D11VertexShader> m_lineVertexShader;
	ComPtr<ID3D11PixelShader> m_linePixelShader;
};

#endif
