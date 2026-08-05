#pragma once
#include "VertexBuffer.h"
class VertexArray {
public:
	VertexArray(VertexBuffer vbo,VertexLayout layout);
	~VertexArray();
	void Bind() const;
	void UnBind() const;
	GLuint GetID() const { return m_ID; }
private:
	GLuint m_ID{};
};