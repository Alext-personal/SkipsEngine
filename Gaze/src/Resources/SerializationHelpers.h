#include "Render/Buffer.h"
#include "Render/Texture.h"
#include "Render/Shader.h"
#include <memory>
namespace Gaze {
#pragma pack(push,1)
	struct MeshFileHeader {
		char validation[5];
		uint32_t format_version;
		uint64_t UUID;
		uint32_t buffers_count;
		uint32_t index_count;
		uint32_t submesh_count;
	};
	struct MeshBufferHeader {
		uint32_t vertex_count;
		uint32_t attribute_count;
		uint32_t stride;
	};
	struct SubMeshEntry {
		uint32_t indexCount;
		uint32_t indexOffset;
		uint32_t materialIndex;
	};
	struct VertexBufferAttributeEntry {
		AttributeDataType type;
		uint32_t size;
		bool normalized;
		uint32_t offset;
	};
	struct TextureFileHeader {
		char validation[8];
		PixelDataFormat format;
		uint32_t width, height;
		uint8_t mipCount;
		uint32_t dataSize;
	};
	struct MipDataPacked {
		uint32_t width, height, offset, byteSize;
	};
	struct ShaderFileHeader {
		char validation[7];
		uint32_t shaderCount;
	};
	struct ShaderDataPacked {
		ShaderType type;
		uint32_t size;
	};
#pragma pack(pop)
}