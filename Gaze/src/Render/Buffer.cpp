#include "pch.h"
#include "Render/Buffer.h"
namespace Gaze {
	VertexBuffer::VertexBuffer(const std::vector<float>& data) {
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
	}
	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}
	ElementBuffer::ElementBuffer(const std::vector<uint32_t>& data) {
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
	}
	ElementBuffer::~ElementBuffer() {
		glDeleteBuffers(1, &m_ID);
	}
	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding) {
		glCreateBuffers(1, &m_ID);
		glNamedBufferData(m_ID, size, nullptr, GL_DYNAMIC_DRAW);
		Bind(binding);
	}

	void UniformBuffer::Bind(uint32_t binding) {
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_ID);
	}
	UniformBuffer::~UniformBuffer() {
		glDeleteBuffers(1, &m_ID);
	}
	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
		glNamedBufferSubData(m_ID, offset, size, data);
	}
}