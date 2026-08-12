#include "Render/Buffer.h"
VertexBuffer::VertexBuffer(const std::vector<float>& data) {
	glGenBuffers(1, &m_ID);
	Bind();
	glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STATIC_DRAW);
	UnBind();
}
void VertexBuffer::Bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}
void VertexBuffer::UnBind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &m_ID);
}

ElementBuffer::ElementBuffer(const std::vector<uint32_t>& data) {
	glGenBuffers(1, &m_ID);
	Bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(uint32_t), data.data(), GL_STATIC_DRAW);
	UnBind();
}
void ElementBuffer::Bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}
void ElementBuffer::UnBind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
ElementBuffer::~ElementBuffer() {
	glDeleteBuffers(1, &m_ID);
}