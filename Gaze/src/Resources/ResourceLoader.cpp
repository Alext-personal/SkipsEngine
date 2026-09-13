#include "pch.h"
#include "Resources/ResourceLoader.h"
#include "Render/Primitives/Primitives.h"
#include "Render/Mesh.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Resources/Prefab.h"
#include "Resources/SerializationHelpers.h"
namespace YAML {
	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(YAML::EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}
	};
}
namespace Gaze{
	MeshData ResourceLoader::LoadMesh(const std::filesystem::path& filepath) {
		if (filepath.extension() != ".gmesh")
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
		if (!std::filesystem::exists(filepath) || filepath.extension() != ".gtex")
		{
			LOG_ERROR("${} :INVALID  file : not .gtex | file corrupted", filepath);
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
		if (strcmp(header.validation,"Texture")) {
			LOG_ERROR("${} :INVALID .gtex file | file corrupted", filepath);
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
		file.read(reinterpret_cast<char*>(returnedData.data.data()), header.dataSize);
		return returnedData;
	}
	Prefab ResourceLoader::LoadPrefab(const std::filesystem::path& filepath) {
		return Prefab();
	}
	MaterialData ResourceLoader::LoadMaterial(const std::filesystem::path& filepath) {
		if (!std::filesystem::exists(filepath) || filepath.extension() != ".gmat")
		{
			LOG_ERROR("${} :INVALID  file : not .gmat | file corrupted", filepath);
			return Material::GetFallbackMaterial();
		}
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return Material::GetFallbackMaterial();
		}
		if (!file["Material"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return Material::GetFallbackMaterial();
		}
		file = file["Material"];
		if (!file["Shader"] || !file["Textures"] || !file["Tint"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return Material::GetFallbackMaterial();
		}
		MaterialData materialData;
		materialData.shader = UUID(file["Shader"].as<uint64_t>(), true);
		materialData.albedoTexture = UUID(file["Textures"]["Albedo"].as<uint64_t>(), true);
		materialData.tint = file["Tint"].as<glm::vec4>();
		return materialData;
	}
	std::vector<ShaderData> ResourceLoader::LoadShader(const std::filesystem::path& filepath) {
		if (!std::filesystem::exists(filepath) || filepath.extension() != ".gs")
		{
			LOG_ERROR("${} :INVALID  file : not .gs | file corrupted", filepath);
			return Shader::GetFallbackShader();
		}
		std::ifstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR("${} : could not open file", filepath);
			return Shader::GetFallbackShader();
		}
		std::vector<ShaderData> shaders;
		ShaderFileHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (strcmp(header.validation, "Shader")) {
			LOG_ERROR("${} :INVALID .gs file | file corrupted", filepath);
			return Shader::GetFallbackShader();
		}
		shaders.resize(header.shaderCount);
		for (uint32_t index = 0; index < header.shaderCount; ++index)
		{
			ShaderDataPacked shader;
			file.read(reinterpret_cast<char*>(&shader), sizeof(shader));
			shaders[index].type = shader.type;
			shaders[index].src.resize(shader.size);
			file.read(reinterpret_cast<char*>(shaders[index].src.data()), shader.size);
		}
		return shaders;
	}
}