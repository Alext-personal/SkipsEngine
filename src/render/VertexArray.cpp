#include "Render/VertexArray.h"
static GLenum AttributeDataTypeToGLenum(AttributeDataType type) {
	switch (type) {
	case AttributeDataType::Float: return GL_FLOAT;
	case AttributeDataType::Float2: return GL_FLOAT;
	case AttributeDataType::Float3: return GL_FLOAT;
	case AttributeDataType::Float4: return GL_FLOAT;

	case AttributeDataType::Int: return GL_INT;
	case AttributeDataType::Int2: return GL_INT;
	case AttributeDataType::Int3: return GL_INT;
	case AttributeDataType::Int4: return GL_INT;

	case AttributeDataType::Bool: return GL_UNSIGNED_BYTE;
	}
}
VertexArray::VertexArray() : m_lastAttributeIndex(0) {
	glGenVertexArrays(1, &m_ID);
}
VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_ID);
}
void VertexArray::AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout) {
	Bind();
	buffer.Bind();
	for (const auto& attribute : layout.GetAttributes())
	{
		glEnableVertexAttribArray(m_lastAttributeIndex);
		if (AttributeDataTypeToGLenum(attribute.type) == GL_FLOAT) {
			glVertexAttribPointer(m_lastAttributeIndex,
								attribute.GetCountByType(),
								AttributeDataTypeToGLenum(attribute.type),
								attribute.normalized,
								layout.GetStride(),
								(void*)attribute.offset);
		}
		else {
			glVertexAttribIPointer(m_lastAttributeIndex,
								attribute.GetCountByType(),
								AttributeDataTypeToGLenum(attribute.type),
								layout.GetStride(),
								(void*)attribute.offset);
		}

		++m_lastAttributeIndex;
	}
	buffer.UnBind();
	UnBind();
}
void VertexArray::SetElementBuffer(const ElementBuffer& buffer) {
	Bind();
	buffer.Bind();
	UnBind();
}
void VertexArray::Bind() const {
	glBindVertexArray(m_ID);
}
void VertexArray::UnBind() const {
	glBindVertexArray(0);
}