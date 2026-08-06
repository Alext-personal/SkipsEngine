#pragma once
#include "VertexBuffer.h"
class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout);

	void Bind() const;
	void UnBind() const;
private:
	uint32_t m_ID{};
};