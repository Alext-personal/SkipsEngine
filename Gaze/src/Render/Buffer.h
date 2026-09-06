#pragma once
#include <vector>
namespace Gaze {
	enum class AttributeDataType :uint8_t{
		None = 0, Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool
	};
	static uint32_t GetSizeByType(AttributeDataType type) {
		switch (type) {
		case AttributeDataType::Float: return 4;
		case AttributeDataType::Float2: return 4 * 2;
		case AttributeDataType::Float3: return 4 * 3;
		case AttributeDataType::Float4: return 4 * 4;

		case AttributeDataType::Int: return 4;
		case AttributeDataType::Int2: return 4 * 2;
		case AttributeDataType::Int3: return 4 * 3;
		case AttributeDataType::Int4: return 4 * 4;

		case AttributeDataType::Bool: return 1;
		}
	}
	static std::string AttributeDataTypeToString(AttributeDataType type) {
		switch (type) {
		case AttributeDataType::Float: return "Float";
		case AttributeDataType::Float2: return "Float2";
		case AttributeDataType::Float3: return "Float3";
		case AttributeDataType::Float4: return "Float4";

		case AttributeDataType::Int: return "Int";
		case AttributeDataType::Int2: return "Int2";
		case AttributeDataType::Int3: return "Int3";
		case AttributeDataType::Int4: return "Int4";

		case AttributeDataType::Bool: return "Bool";
		}
		ENGINE_ASSERT(0, "INVALID STRING ATTRIBUTE DATA TYPE");
	}
	static AttributeDataType StringToAttributeDataType(const std::string& str) {
		if (str == "Float") return AttributeDataType::Float;
		if (str == "Float2") return AttributeDataType::Float2;
		if (str == "Float3") return AttributeDataType::Float3;
		if (str == "Float4") return AttributeDataType::Float4;
		if (str == "Int") return AttributeDataType::Int;
		if (str == "Int2") return AttributeDataType::Int2;
		if (str == "Int3") return AttributeDataType::Int3;
		if (str == "Int4") return AttributeDataType::Int4;
		if (str == "Bool") return AttributeDataType::Bool;
		ENGINE_ASSERT(0, "INVALID STRING ATTRIBUTE DATA TYPE");
	}
	struct VertexBufferAttribute {
		AttributeDataType type;
		uint32_t size;
		bool normalized;
		uint32_t offset;

		VertexBufferAttribute(AttributeDataType _type, bool _normalized = 0)
			: type(_type), size(GetSizeByType(_type)),
			normalized(_normalized), offset(0) {
		}
		VertexBufferAttribute(AttributeDataType _type, uint32_t _size,bool _norm,uint32_t _ofs)
			: type(_type),size(_size),normalized(_norm),offset(_ofs){}
		VertexBufferAttribute() = default;

		uint32_t GetCountByType() const {
			switch (type) {
			case AttributeDataType::Float: return 1;
			case AttributeDataType::Float2: return 2;
			case AttributeDataType::Float3: return 3;
			case AttributeDataType::Float4: return 4;

			case AttributeDataType::Int: return 1;
			case AttributeDataType::Int2: return 2;
			case AttributeDataType::Int3: return 3;
			case AttributeDataType::Int4: return 4;

			case AttributeDataType::Bool: return 1;
			}
		}
	};
	class VertexLayout {
	public:
		VertexLayout() = default;
		void Add(AttributeDataType type, bool normalized = 0) {
			VertexBufferAttribute a(type, normalized);
			m_attributes.push_back(a);
			m_updateAttributes();
		}
		uint32_t GetStride() const { return m_stride; }
		std::vector<VertexBufferAttribute>& GetAttributes() { return m_attributes; }
		const std::vector<VertexBufferAttribute>& GetAttributes() const { return m_attributes; }
		void UpdateAttributes() { m_updateAttributes(); }
	private:
		std::vector<VertexBufferAttribute> m_attributes{};
		uint32_t m_stride{};

		void m_updateAttributes() {
			uint32_t _offset = 0;
			m_stride = 0;
			for (auto& a : m_attributes) {
				a.offset = _offset;
				_offset += a.size;
				m_stride += a.size;
			}
		}
	};

	class VertexBuffer {
	public:
		VertexBuffer(const std::vector<float>& data);
		~VertexBuffer();
		uint32_t GetID() const { return m_ID; }
	private:
		uint32_t m_ID{};
	};
	class ElementBuffer {
	public:
		ElementBuffer(const std::vector<uint32_t>& data);
		~ElementBuffer();
		uint32_t GetID() const { return m_ID; }
	private:
		uint32_t m_ID{};
	};
	class UniformBuffer {
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		~UniformBuffer();
		void SetData(const void* data, uint32_t size, uint32_t offset);
		void Bind(uint32_t binding);
	private:
		uint32_t m_ID{};
	};
}
