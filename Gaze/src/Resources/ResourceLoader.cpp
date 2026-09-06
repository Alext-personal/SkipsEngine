#include "pch.h"
#include "Resources/ResourceLoader.h"
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Resources/Prefab.h"
#include "Resources/SerializationHelpers.h"
namespace Gaze{
	MeshData ResourceLoader::LoadMesh(const std::filesystem::path& filepath) {
		if (filepath.extension() != ".mesh")
		{
			LOG_ERROR("${} :INVALID file, not .mesh | file corrupted", filepath);
			return Primitives::LoadPrimitiveByType(PrimitiveType::Cube);
		}
		std::ifstream file(filepath, std::ios::binary);
		MeshFileHeader header;
		MeshData returnedData;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (header.validation[0] != 'M' || header.validation[1] != 'e' || header.validation[2] != 's' || header.validation[3] != 'h') {
			LOG_ERROR("${} :INVALID .mesh file | file corrupted", filepath);
			return Primitives::LoadPrimitiveByType(PrimitiveType::Cube);
		}
		//submeshes load
		std::vector<SubMeshEntry> subMeshesPacked;
		subMeshesPacked.resize(header.submesh_count);
		file.read(reinterpret_cast<char*>(subMeshesPacked.data()), header.submesh_count * sizeof(SubMeshEntry));
		for (auto& submesh : subMeshesPacked) {
			returnedData.subMeshes.push_back({ submesh.indexCount, submesh.indexOffset, submesh.materialIndex });
		}
		//indices load
		returnedData.indices.resize(header.index_count);
		file.read(reinterpret_cast<char*>(returnedData.indices.data()), header.index_count * sizeof(uint32_t));

		//bufferdata load
		returnedData.bufferData.resize(header.buffers_count);
		for (uint32_t index = 0; index < header.buffers_count; ++index) {
			MeshBufferHeader bufferHeader;
			file.read(reinterpret_cast<char*>(&bufferHeader), sizeof(bufferHeader));
			returnedData.bufferData[index].data.resize(bufferHeader.vertex_count);
			file.read(reinterpret_cast<char*>(returnedData.bufferData[index].data.data()), bufferHeader.vertex_count * sizeof(float));
			std::vector<VertexBufferAttributeEntry> attributesPacked;
			attributesPacked.resize(bufferHeader.attribute_count);
			file.read(reinterpret_cast<char*>(attributesPacked.data()), bufferHeader.attribute_count * sizeof(VertexBufferAttributeEntry));
			for (auto& attr : attributesPacked) {
				returnedData.bufferData[index].layout.GetAttributes().push_back({ attr.type,attr.size,attr.normalized,attr.offset });
			}
			returnedData.bufferData[index].layout.UpdateAttributes();
		}
		return returnedData;
	}
	TextureData ResourceLoader::LoadTexture(const std::filesystem::path& filepath) {
		if (filepath.extension() != ".tex")
		{
			LOG_ERROR("${} :INVALID  file : not .tex | file corrupted", filepath);
			return Texture::GetFallbackTexture();
		}
		std::ifstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR("${} : could not open file", filepath);
			return Texture::GetFallbackTexture();
		}
		TextureFileHeader header;
		TextureData returnedData;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (header.validation[0] != 'T' || header.validation[1] != 'e' || header.validation[2] != 'x' || header.validation[3] != 't' || header.validation[4] != 'u' || header.validation[5] != 'r' || header.validation[6] != 'e') {
			LOG_ERROR("${} :INVALID .tex file | file corrupted", filepath);
			return Texture::GetFallbackTexture();
		}
		returnedData.format = header.format;
		returnedData.width = header.width;
		returnedData.height = header.height;
		returnedData.mipCount = header.mipCount;

		//read mips data
		std::vector<MipDataPacked> packedMips;
		packedMips.resize(header.mipCount);
		file.read(reinterpret_cast<char*>(packedMips.data()), header.mipCount * sizeof(MipDataPacked));
		uint32_t expectedDataSize = packedMips.empty() ? 0 : (packedMips.back().offset + packedMips.back().byteSize);
		if (expectedDataSize != header.dataSize) {
			LOG_ERROR("${} : mip data size mismatch, file corrupted", filepath);
			return Texture::GetFallbackTexture();
		}
		for (auto& mip : packedMips) {
			returnedData.mips.push_back({ mip.width,mip.height,mip.offset,mip.byteSize });
		}
		//read texturedata
		returnedData.data.resize(header.dataSize);
		file.read(reinterpret_cast<char*>(returnedData.data.data()), header.dataSize * sizeof(uint8_t));
		return returnedData;
	}
	Prefab ResourceLoader::LoadPrefab(const std::filesystem::path filepath) {

	}
}