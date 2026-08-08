#pragma once
#include "Render/Buffer.h"
class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout);
	void SetElementBuffer(const ElementBuffer& buffer);

	void Bind() const;
	void UnBind() const;
private:
	uint32_t m_ID{};
	uint32_t m_lastAttributeIndex;
};