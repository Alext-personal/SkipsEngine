#include "Render/VertexArray.h"
namespace Gaze {
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
		glCreateVertexArrays(1, &m_ID);
	}
	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_ID);
	}
	void VertexArray::AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout) {
		uint32_t bindingIndex = m_lastAttributeIndex;
		glVertexArrayVertexBuffer(m_ID, bindingIndex, buffer.GetID(), 0, layout.GetStride());
		for (const auto& attribute : layout.GetAttributes())
		{
			glEnableVertexArrayAttrib(m_ID, m_lastAttributeIndex);
			if (AttributeDataTypeToGLenum(attribute.type) == GL_FLOAT) {
				glVertexArrayAttribFormat(m_ID,
					m_lastAttributeIndex,
					attribute.GetCountByType(),
					AttributeDataTypeToGLenum(attribute.type),
					attribute.normalized,
					attribute.offset);
			}
			else {
				glVertexArrayAttribIFormat(m_ID,
					m_lastAttributeIndex,
					attribute.GetCountByType(),
					AttributeDataTypeToGLenum(attribute.type),
					attribute.offset);
			}
			glVertexArrayAttribBinding(m_ID, m_lastAttributeIndex, bindingIndex);
			++m_lastAttributeIndex;
		}

	}
	void VertexArray::SetElementBuffer(const ElementBuffer& buffer) {
		glVertexArrayElementBuffer(m_ID, buffer.GetID());
	}
	void VertexArray::Bind() const {
		glBindVertexArray(m_ID);
	}
	void VertexArray::UnBind() const {
		glBindVertexArray(0);
	}
}