#pragma once
#include <filesystem>
#include <utility>
#include <fstream>
#include <assimp/Importer.hpp>   
#include <assimp/scene.h>           
#include <assimp/postprocess.h> 
#include "Render/Mesh.h"
class ModelLoader {
public:
	static MeshData LoadModel(const std::filesystem::path& filepath) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath.string(),
			aiProcess_Triangulate |
			aiProcess_GenNormals  |
			aiProcess_GenUVCoords);
		if (scene == nullptr || scene->mNumMeshes == 0) {
			LOG_WARNING("Failed to import model");
			MeshData nullData;
			return nullData;
		}
		MeshData loadedData{};
		loadedData.bufferData[0].layout.Add(AttributeDataType::Float3); // pos mandatory
		bool hasNormals = scene->mMeshes[0]->HasNormals();
		if(hasNormals)
			loadedData.bufferData[0].layout.Add(AttributeDataType::Float3);
		bool hasTextureCoords = scene->mMeshes[0]->HasTextureCoords(0);
		if(hasTextureCoords)
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
					else if(hasNormals)
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
			Log::INFO("subMesh offset : ", currentOffset, " subMesh Count: ", indexCount);
			currentOffset += indexCount;
			currentVertexOffset += mesh->mNumVertices;
		}

		return loadedData;
			
	}
};