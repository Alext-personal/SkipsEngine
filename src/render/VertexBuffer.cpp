#include "render/VertexBuffer.h"
VertexBuffer::VertexBuffer(const void* data,size_t size){
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
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