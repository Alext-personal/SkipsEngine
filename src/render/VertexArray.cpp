#include "render/VertexArray.h"
static GLenum AttributeDataTypeToGLenum(AttributeDataType type) {
	switch (type) {
	case AttributeDataType::Float: return GL_FLOAT;
	case AttributeDataType::Float2: return GL_FLOAT;
	case AttributeDataType::Float3: return GL_FLOAT;
	case AttributeDataType::Float4: return GL_FLOAT;

	case AttributeDataType::Mat3: return GL_FLOAT;
	case AttributeDataType::Mat4: return GL_FLOAT;

	case AttributeDataType::Int: return GL_INT;
	case AttributeDataType::Int2: return GL_INT;
	case AttributeDataType::Int3: return GL_INT;
	case AttributeDataType::Int4: return GL_INT;

	case AttributeDataType::Bool: return GL_UNSIGNED_BYTE;
	default:
		Log::ERROR("Invalid Type");
	}
}
VertexArray::VertexArray() {
	glGenVertexArrays(1, &m_ID);
	glBindVertexArray(m_ID);
}
VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_ID);
}
void VertexArray::AddVertexBuffer(VertexBuffer buffer, VertexLayout layout) {
	buffer.Bind();
	uint32_t index = 0;
	for (const auto& attribute : layout.GetAttributes())
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, 
							attribute.GetCountByType(),
							AttributeDataTypeToGLenum(attribute.type), 
							attribute.normalized, 
							layout.GetStride(), 
							(void*)attribute.offset);
		++index;
	}
}
void VertexArray::Bind() const {
	glBindVertexArray(m_ID);
}
void VertexArray::UnBind() const {
	glBindVertexArray(0);
}