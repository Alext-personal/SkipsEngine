#pragma once
#include <vector>
#include "core/Log.h"
#include <glad/glad.h>
enum class AttributeDataType {
	None = 0, Float, Float2, Float3, Float4, Mat3 , Mat4, Int, Int2, Int3, Int4, Bool // mat3 = 9 floats, mat4 = 16 floats
};
static size_t GetSizeByType(AttributeDataType type) {
	switch (type) {
	case AttributeDataType::Float: return 4;
	case AttributeDataType::Float2: return 4*2;
	case AttributeDataType::Float3: return 4*3;
	case AttributeDataType::Float4: return 4*4;

	case AttributeDataType::Int: return 4;
	case AttributeDataType::Int2: return 4 * 2;
	case AttributeDataType::Int3: return 4 * 3;
	case AttributeDataType::Int4: return 4 * 4;

	case AttributeDataType::Mat3: return 4 * 3 * 3; 
	case AttributeDataType::Mat4: return 4 * 4 * 4;

	case AttributeDataType::Bool: return 1;
	default:
		Log::ERROR("Invalid Type");
	}
}
struct VertexBufferAttribute {
	AttributeDataType type;
	size_t size;
	bool normalized;
	size_t offset;

	VertexBufferAttribute(AttributeDataType _type, bool _normalized = 0) : type(_type), size(GetSizeByType(_type)){}
};
class VertexLayout {
public:
	VertexLayout() = default;
	void Add(AttributeDataType type,bool normalized = 0) {
		VertexBufferAttribute a(type, normalized);
		m_attributes.push_back(a);
		m_updateAttributes();
	}
	size_t GetStride() const { return m_stride; }
private:
	std::vector<VertexBufferAttribute> m_attributes{};
	size_t m_stride{};

	void m_updateAttributes() {
		size_t _offset = 0;
		m_stride = 0;
		for (auto& a : m_attributes) {
			a.offset = _offset;
			_offset += a.size;
		}
		m_stride = _offset;
	}
};

class VertexBuffer {
public:
	VertexBuffer(const void* data,size_t size);
	~VertexBuffer();
	void Bind() const;
	void UnBind() const;

	GLuint GetID() const { return m_ID; }
private:
	GLuint m_ID{};
};
