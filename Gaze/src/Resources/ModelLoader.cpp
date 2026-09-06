#include "pch.h"
#include "Resources/ModelLoader.h"
#include "Render/Primitives/Primitives.h"
#include "Resources/SerializationHelpers.h"
namespace Gaze {
	 MeshData ModelLoader::LoadModel(const std::filesystem::path& filepath) {
		auto start = GetTime();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath.string(),
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords);
		auto t1 = GetTime();
		LOG_INFO("ASSIMP TOOK: ${} ", t1 - start);
		if (scene == nullptr || scene->mNumMeshes == 0) {
			LOG_ERROR("Failed to import model");
			MeshData nullData;
			return nullData;
		}
		MeshData loadedData{};
		loadedData.bufferData[0].layout.Add(AttributeDataType::Float3); // pos mandatory
		bool hasNormals = scene->mMeshes[0]->HasNormals();
		if (hasNormals)
			loadedData.bufferData[0].layout.Add(AttributeDataType::Float3);
		bool hasTextureCoords = scene->mMeshes[0]->HasTextureCoords(0);
		if (hasTextureCoords)
			loadedData.bufferData[0].layout.Add(AttributeDataType::Float2);

		uint32_t currentOffset{};
		uint32_t currentVertexOffset{}; // for indices;
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];
			auto& pos = mesh->mVertices;
			auto& normals = mesh->mNormals;
			auto& textureCoords = mesh->mTextureCoords;
			bool _hasNormals = mesh->HasNormals();
			bool _hasTextureCoords = mesh->HasTextureCoords(0);
			for (uint32_t index = 0; index < mesh->mNumVertices; ++index) {
				loadedData.bufferData[0].data.push_back(pos[index].x);
				loadedData.bufferData[0].data.push_back(pos[index].y);
				loadedData.bufferData[0].data.push_back(pos[index].z);
				if (_hasNormals)
				{
					loadedData.bufferData[0].data.push_back(normals[index].x);
					loadedData.bufferData[0].data.push_back(normals[index].y);
					loadedData.bufferData[0].data.push_back(normals[index].z);
				} //todo fix issue, if one mesh has no normals/texcoords buffer should skip 
				else if (hasNormals)
				{
					loadedData.bufferData[0].data.emplace_back();
					loadedData.bufferData[0].data.emplace_back();
					loadedData.bufferData[0].data.emplace_back();
				}
				if (_hasTextureCoords) {
					loadedData.bufferData[0].data.push_back(textureCoords[0][index].x);
					loadedData.bufferData[0].data.push_back(textureCoords[0][index].y);
				}
				else if (hasTextureCoords) {
					loadedData.bufferData[0].data.emplace_back();
					loadedData.bufferData[0].data.emplace_back();
				}
			}
			auto& faces = mesh->mFaces;
			uint32_t indexCount = 0;
			for (uint32_t index = 0; index < mesh->mNumFaces; ++index) {
				auto& face = faces[index];
				for (uint32_t index2 = 0; index2 < face.mNumIndices; ++index2)
				{
					loadedData.indices.push_back(face.mIndices[index2] + currentVertexOffset); //assimp face indices local to current face
					++indexCount;
				}
			}
			loadedData.subMeshes.push_back({ indexCount,currentOffset });
			LOG_INFO("subMesh offset: ${} subMesh Count: ${} ", currentOffset, indexCount);
			currentOffset += indexCount;
			currentVertexOffset += mesh->mNumVertices;
		}
		auto end = GetTime();
		LOG_INFO("Mesh loader took: ${} ", end - start);
		return loadedData;

	} // editor
	 void ModelLoader::ExportMesh(const MeshData& mesh, const UUID& uuid) {
		 std::filesystem::path filepath = GetCurrentPath() / "cookedAssets" / uuid.ToString();
		 filepath += ".mesh";
		 std::ofstream file(filepath, std::ios::binary);
		 MeshFileHeader header;
		 memcpy(header.validation, "Mesh", 4);
		 header.format_version = 1;
		 header.UUID = uuid;
		 header.buffers_count = mesh.bufferData.size();
		 header.index_count = mesh.indices.size();
		 header.submesh_count = mesh.subMeshes.size();
		 file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		 std::vector<SubMeshEntry> subMeshesPacked;
		 for (auto& submesh : mesh.subMeshes) {
			 subMeshesPacked.push_back({ submesh.indexCount,submesh.indexOffset,submesh.materialIndex });
		 }
		 file.write(reinterpret_cast<const char*>(subMeshesPacked.data()), mesh.subMeshes.size() * sizeof(SubMeshEntry));
		 file.write(reinterpret_cast<const char*>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t));
		 for (uint32_t index = 0; index < mesh.bufferData.size(); ++index) {
			 MeshBufferHeader bufferHeader;
			 bufferHeader.attribute_count = mesh.bufferData[index].layout.GetAttributes().size();
			 LOG_WARNING("ATTRIBUTE_COUNT : ${}", bufferHeader.attribute_count);
			 int index1 = 0;
			 for (auto& m : mesh.bufferData[index].layout.GetAttributes())
				 LOG_WARNING("ATTRIBUTE [${}] : OFFSET : ${}", index1++, m.offset);
			 bufferHeader.vertex_count = mesh.bufferData[index].data.size();
			 bufferHeader.stride = mesh.bufferData[index].layout.GetStride();
			 file.write(reinterpret_cast<const char*>(&bufferHeader), sizeof(bufferHeader));
			 file.write(reinterpret_cast<const char*>(mesh.bufferData[index].data.data()), bufferHeader.vertex_count * sizeof(float));
			 std::vector<VertexBufferAttributeEntry> attributesPacked;
			 for (auto& attr : mesh.bufferData[index].layout.GetAttributes())
				 attributesPacked.push_back({ attr.type,attr.size,attr.normalized,attr.offset });
			 file.write(reinterpret_cast<const char*>(attributesPacked.data()), bufferHeader.attribute_count * sizeof(VertexBufferAttributeEntry));
		 }
	 } // editor
}