#include "pch.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/SerializationHelpers.h"
#include "Resources/ResourceManager.h"
#include "Render/Mesh.h"
#include "Core/Helpers.h"
#include <xxhash.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace Gaze {
	inline static std::vector<aiTextureType> TextureTypes
	{ aiTextureType_BASE_COLOR,aiTextureType_DIFFUSE,aiTextureType_METALNESS,
		aiTextureType_DIFFUSE_ROUGHNESS,aiTextureType_NORMALS,aiTextureType_OPACITY,
		aiTextureType_SPECULAR,aiTextureType_SHININESS
	};
	struct NodeData { // i consider materials nodes
		UUID id = 0;
		std::filesystem::path nodePath{};
		aiMatrix4x4 transform{};
		UUID parent = 0;
		UUID meshID = 0;
		MeshData mesh{};
		std::vector<UUID> materials{};
	};
	struct MaterialNodeData {
		UUID id = 0;
		std::string name;
		std::string shaderType; // for material creation to choose the default shader the material uses
		std::unordered_map<aiTextureType, UUID> textures;
		int indexInMaterials = -1;
	};
	struct TextureNodeData {
		UUID id;
		std::filesystem::path relativePath ="SHITASSFUCKYOU";
		int textureSlot = -10; // -10 = material isn't in use anymore
	};
	inline YAML::Node BuildMaterialFile(const aiScene* scene, const MaterialNodeData& data) {
		YAML::Node root;
		YAML::Node materialNode;

		if (data.indexInMaterials  >= scene->mNumMaterials || data.indexInMaterials < 0) {
			LOG_ERROR("[MATERIAL BUILDER] material '${}' not found in scene", data.name);
			return root;
		}
		aiMaterial* mat = scene->mMaterials[data.indexInMaterials];
		materialNode["Shader"] = ReservedUUID::DEFAULTSHADER.Get();
		aiColor4D baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		if (mat->Get(AI_MATKEY_BASE_COLOR, baseColor) != aiReturn_SUCCESS) {
			aiColor3D diffuse{ 1.0f, 1.0f, 1.0f };
			mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			float opacity = 1.0f;
			mat->Get(AI_MATKEY_OPACITY, opacity);
			baseColor = { diffuse.r, diffuse.g, diffuse.b, opacity };
		}
		YAML::Node tint;
		tint.push_back(baseColor.r);
		tint.push_back(baseColor.g);
		tint.push_back(baseColor.b);
		tint.push_back(baseColor.a);
		materialNode["Tint"] = tint;

		// shader-specific params
		if (data.shaderType == "Pbr") {
			float metallic = 0.0f, roughness = 1.0f;
			mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			materialNode["Metallic"] = metallic;
			materialNode["Roughness"] = roughness;
		}
		else if (data.shaderType == "Blinn") {
			float shininess = 32.0f;
			aiColor3D specular{ 1.0f, 1.0f, 1.0f };
			mat->Get(AI_MATKEY_SHININESS, shininess);
			mat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
			YAML::Node specNode;
			specNode.push_back(specular.r);
			specNode.push_back(specular.g);
			specNode.push_back(specular.b);
			materialNode["Specular"] = specNode;
			materialNode["Shininess"] = shininess;
		}

		YAML::Node tex;
		for (auto& [slot, texture] : data.textures) {
			std::string slotString = aiTextureTypeToString(slot);
			if (slotString == "Diffuse" || slotString == "BaseColor")
				slotString = "Albedo";
			tex[slotString] = texture.Get();
		}
		materialNode["Textures"] = tex;

		root["Material"] = materialNode;
		return root;
	}
	inline uint64_t SaveSnapshot(void* snapshot, uint32_t size,uint64_t oldSnapshot,std::string name) {
		uint64_t newSnapshotHash = AssetImporter::GetContentHash(snapshot, size);
		if (newSnapshotHash != oldSnapshot) {
			std::filesystem::path snapshotPath = (GetCurrentPath() / "Library" / "Snapshots" / name).lexically_normal();
			snapshotPath += ".bin";
			std::filesystem::create_directories(snapshotPath.parent_path());
			std::ofstream file(snapshotPath, std::ios::binary);
			if (!(file.is_open()))
			{
				LOG_ERROR("failure on snapshot creation : ${}", snapshotPath);
				return 0;
			}

			file.write(reinterpret_cast<const char*>(snapshot), size);
			file.close();
			return newSnapshotHash;
		}
		return oldSnapshot;
	}
	inline bool ModifyMetaImportHash(MetaData& meta) {
		uint64_t hash = AssetImporter::GetContentHash(meta.source);
		if (meta.importSettings != nullptr)
			hash = AssetImporter::GetContentHash(hash, meta.importSettings->GetHash());
		meta.importHash = hash;
		if (meta.isStandalone == false)
			return true;
		YAML::Node metaFile;
		std::filesystem::path metaPath = meta.source;
		metaPath += ".meta";
		try {
			metaFile = YAML::LoadFile(metaPath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", metaPath, e.what());
			return false;
		}
		if (!metaFile)
		{
			LOG_ERROR("MetaFile corurpted : {}", metaPath);
			return false;
		}
		metaFile["ImportHash"] = hash;
		std::ofstream file(metaPath);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to open meta file for writing: {}", metaPath);
			return false;
		}

		file << metaFile;
		file.close();
		return true;
	}
	inline void ExtractPrefabNodeIDs(const YAML::Node& node, std::unordered_map<std::filesystem::path, NodeData>& outMesh) {
		YAML::Node entity = node["Entity"];
		if (!entity || !entity["NodePath"] || !entity["UUID"])
			return;
		outMesh[entity["NodePath"].as<std::string>()].id = entity["UUID"].as<uint64_t>();
		for (auto child : entity["Children"])
			ExtractPrefabNodeIDs(child, outMesh);
	}

	inline void ExtractPrefabNodeIDs(const std::filesystem::path& filepath, std::unordered_map<std::filesystem::path, NodeData>& outMesh) {
		if (!std::filesystem::exists(filepath))
			return;
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return;
		}
		YAML::Node prefab = file["Prefab"];
		if (!prefab)
		{
			LOG_ERROR("${}  Invalid .gprefab file  ", filepath);
			return;
		}
		ExtractPrefabNodeIDs(prefab, outMesh);
		
	}
	inline void LoadSourceMeta(std::unordered_map<std::filesystem::path, NodeData>& outMesh,std::unordered_map<std::string,MaterialNodeData>& outMat
		,std::unordered_map<std::string,TextureNodeData>& outTex,UUID& prefabID,const MetaData& data) {
		for (auto& dependency : data.generatedDependencies) {
			MaterialNodeData matNode;
			NodeData meshNode;
			std::string nodePathStr = dependency.nodePath.string();
			switch (dependency.type) {
				case AssetType::Material:
					matNode.id = dependency.id;
					matNode.name = nodePathStr;
					outMat[matNode.name] = matNode;
					break;
				case AssetType::Texture:
					outTex[nodePathStr].id = dependency.id;
					break;
				case AssetType::Mesh:
					meshNode.meshID = dependency.id;
					meshNode.nodePath = dependency.nodePath;
					outMesh[dependency.nodePath] = meshNode;
					break;
				case AssetType::Prefab:
					prefabID = dependency.id;
					ExtractPrefabNodeIDs(dependency.nodePath, outMesh); // for a prefab nodePath is just its sourcePath
					break;
				default:
					LOG_ERROR("[MODEL IMPORTER] ASSET TYPE UNKNOWN, REGENERATING ASSET");
					continue;
			}
			
		}
	}
	inline void GetNodePath(const aiNode* node,std::filesystem::path& out) {
		if (node == nullptr)
			return;
		if (node->mParent == nullptr) {
			out = node->mName.C_Str();
		}
		else
		{
			GetNodePath(node->mParent, out);
			out = out / node->mName.C_Str();
		}
	}
	inline void ExtractNodes(const aiScene* scene,const aiNode* node,std::unordered_map<std::filesystem::path,NodeData>& out,bool& dirty) {
		std::filesystem::path nodePath{};
		GetNodePath(node,nodePath);
		NodeData loadedNode;
		if (out.find(nodePath) != out.end()) {
			loadedNode = out[nodePath];
			if (loadedNode.id == 0)
				loadedNode.id = UUID();
			loadedNode.nodePath = nodePath;
		}
		else
		{
			loadedNode.nodePath = nodePath;
			loadedNode.id = UUID();
			if (node->mNumMeshes != 0) {
				loadedNode.meshID = UUID();
				dirty = true;
			}
			else {
				loadedNode.meshID = 0;
				std::filesystem::path parentNodePath{};
				GetNodePath(node->mParent, parentNodePath);
				if (parentNodePath.empty())
					loadedNode.parent = 0;
				else
					loadedNode.parent = out[parentNodePath].id; //guaranteed to be generated
				loadedNode.transform = node->mTransformation;
				out[nodePath] = loadedNode;
				for (uint32_t index = 0; index < node->mNumChildren; ++index) {
					aiNode* child = node->mChildren[index];
					ExtractNodes(scene, child, out,dirty);
				}
				return;
			}
		}
		MeshData loadedData;
		bool attr[3]{}; //0 = has uv
		loadedData.bufferData[0].layout.Add(AttributeDataType::Float3); // pos mandatory
		loadedData.bufferData[0].layout.Add(AttributeDataType::Float3); //normals generated by assimp if not existent
		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(0))
				attr[0] = true;
		}
		if (attr[0])
			loadedData.bufferData[0].layout.Add(AttributeDataType::Float2);
		uint32_t currentOffset{};
		uint32_t currentVertexOffset{}; // for indices;

		for (uint32_t i = 0; i < node->mNumMeshes; ++i) //submesh
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto& pos = mesh->mVertices;
			auto& normals = mesh->mNormals;
			auto& textureCoords = mesh->mTextureCoords;
			bool _hasTextureCoords = mesh->HasTextureCoords(0);
			for (uint32_t index = 0; index < mesh->mNumVertices; ++index) {
				loadedData.bufferData[0].data.push_back(pos[index].x);
				loadedData.bufferData[0].data.push_back(pos[index].y);
				loadedData.bufferData[0].data.push_back(pos[index].z);
				loadedData.bufferData[0].data.push_back(normals[index].x);
				loadedData.bufferData[0].data.push_back(normals[index].y);
				loadedData.bufferData[0].data.push_back(normals[index].z);
				if (_hasTextureCoords) {
					loadedData.bufferData[0].data.push_back(textureCoords[0][index].x);
					loadedData.bufferData[0].data.push_back(textureCoords[0][index].y);
				}
				else if (attr[0]) {
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
					loadedData.indices.push_back(face.mIndices[index2] + currentVertexOffset); //assimp face indices local to current mesh
					++indexCount;
				}
			}
			loadedData.subMeshes.push_back({ indexCount,currentOffset,mesh->mMaterialIndex});
			LOG_INFO("subMesh offset: ${} subMesh Count: ${} ", currentOffset, indexCount);
			currentOffset += indexCount;
			currentVertexOffset += mesh->mNumVertices;
		}
		loadedNode.mesh = loadedData;
		loadedNode.transform = node->mTransformation;
		
		std::filesystem::path parentNodePath{};
		GetNodePath(node->mParent, parentNodePath);
		if (parentNodePath.empty())
			loadedNode.parent = 0;
		else
			loadedNode.parent = out[parentNodePath].id; //guaranteed to be generated
		out[nodePath] = loadedNode;
		for (uint32_t index = 0; index < node->mNumChildren; ++index) {
			aiNode* child = node->mChildren[index];
			ExtractNodes(scene, child, out,dirty);
		}
	}
	inline void ExtractMaterialIDs(const aiScene* scene, std::unordered_map<std::string,MaterialNodeData>& out,std::vector<UUID>& indexes,bool& dirty) {
		std::unordered_map<std::filesystem::path, uint32_t> count;
		for (uint32_t index = 0; index < scene->mNumMaterials; ++index) {
			aiMaterial* mat = scene->mMaterials[index];
			std::string materialName = mat->GetName().C_Str();
			if (materialName.empty())
				materialName = "Material_" + std::to_string(index);
			count[materialName]++;
			if (out.find(materialName) == out.end())
			{
				out[materialName].name = materialName;
				out[materialName].id = UUID();
				dirty = true;
			}
			else if (count[materialName] > 1 )
			{
				LOG_ERROR("DUPLICATE MATERIAL NAME FOUND IN FILE");
				continue;
			}
			uint32_t shading = aiShadingMode_PBR_BRDF;
			out[materialName].indexInMaterials = index;
			mat->Get(AI_MATKEY_SHADING_MODEL, shading);
			switch (shading) {
				case aiShadingMode::aiShadingMode_NoShading:
					out[materialName].shaderType = "Unlit";
					break;

				case aiShadingMode::aiShadingMode_Flat:
					out[materialName].shaderType = "Flat";
					break;

				case aiShadingMode::aiShadingMode_Gouraud:
				case aiShadingMode::aiShadingMode_Phong:
				case aiShadingMode::aiShadingMode_Blinn:
					out[materialName].shaderType = "Blinn";
					break;

				case aiShadingMode::aiShadingMode_Toon:
					out[materialName].shaderType = "Toon";
					break;

				case aiShadingMode::aiShadingMode_OrenNayar:
				case aiShadingMode::aiShadingMode_Minnaert:
				case aiShadingMode::aiShadingMode_CookTorrance:
				case aiShadingMode::aiShadingMode_Fresnel:
				case aiShadingMode::aiShadingMode_PBR_BRDF:
					out[materialName].shaderType = "Pbr";
					break;

				default:
					out[materialName].shaderType = "Pbr";
					break;
			}
			indexes[index] = out[materialName].id;
			
		}
	}
	inline void ExtractTextureIDs(std::filesystem::path relativepath,const aiScene* scene, std::unordered_map<std::string, TextureNodeData>& out,std::unordered_map<std::string,MaterialNodeData>& out2,bool& dirty) {
		for (uint32_t index = 0; index < scene->mNumMaterials; ++index) {
			aiMaterial* mat = scene->mMaterials[index];
			std::string materialName = mat->GetName().C_Str();
			if (materialName.empty())
				materialName = "Material_" + std::to_string(index);
			for (auto& slot : TextureTypes) {
				aiString source;
				
				if (mat->GetTexture(slot, 0, &source) == aiReturn_SUCCESS) {
					const char* sourceStr = source.C_Str();
					if (sourceStr[0] == '*')
					{
						uint32_t textureSlot = std::atoi(sourceStr + 1);
						if (scene->mTextures[textureSlot]->mHeight != 0)
						{
							LOG_ERROR("Embedded texture must be compressed ,uncompressed format not supported ");
							continue;
						}
						std::stringstream ss;
						ss << AssetImporter::GetContentHash((void*)scene->mTextures[textureSlot]->pcData,scene->mTextures[textureSlot]->mWidth);
						std::string key = ss.str();
						if (out.find(key) == out.end()) {
							out[key].id = UUID();
							dirty = true;
						}
						out[key].relativePath = key;
						out[key].textureSlot = textureSlot;
						out2[materialName].textures[slot] = out[key].id;
					}
					else {
						std::filesystem::path path{ sourceStr };
						if (path.is_absolute())
						{
							std::filesystem::path original = path; 
							std::error_code ec;
							std::filesystem::path rel = std::filesystem::relative(original,relativepath.parent_path(), ec);
							if (ec || rel.empty()) {
								LOG_ERROR("${} could not be made relative to model directory", original);
								continue;
							}
							path = rel;
						}
						path = path.lexically_normal();
						std::string pathStr = path.generic_string();
						if (out.find(pathStr) == out.end()) {
							out[pathStr].id = UUID();
							dirty = true;
						}
						out[pathStr].relativePath = pathStr;
						out[pathStr].textureSlot = -1;
						out2[materialName].textures[slot] = out[pathStr].id;
					}
				}
			}
		} 
	}
	inline std::filesystem::path CookMesh(const MeshData& mesh,const UUID& uuid) {
		std::filesystem::path filepath = (GetCurrentPath() / "Library" / "Client" / "Meshes" / uuid.ToString()).lexically_normal();
		filepath += ".gmesh";
		std::filesystem::create_directories(filepath.parent_path());
		std::ofstream file(filepath, std::ios::binary);
		MeshFileHeader header;
		memcpy(header.validation, "Mesh", 5);
		header.format_version = 1;
		header.UUID = uuid.Get();
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
		file.close();
		return filepath;
	}
	std::filesystem::path AssetImporter::ImportModel(const UUID& id)
	{

		std::filesystem::path filepath = m_registry.storage[id].source;
		auto start = GetTime();
		YAML::Node sourceMeta;
		try {
			sourceMeta = YAML::LoadFile(filepath.string() + ".meta");
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return std::filesystem::path();
		}
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath.string(),
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords);
		auto t1 = GetTime();
		LOG_INFO("ASSIMP TOOK: ${} ", t1 - start);
		if (scene == nullptr || scene->mNumMeshes == 0) {
			LOG_ERROR("Failed to import model ${}", filepath);
			return std::filesystem::path();
		}
		std::unordered_map<std::filesystem::path, NodeData>loadedModel;
		std::unordered_map<std::string, TextureNodeData>loadedTextures;
		std::unordered_map<std::string,MaterialNodeData>loadedMaterials;
		std::vector<AssetDependency> generated;
		bool dirtyMeta = false;
		std::vector<UUID> materialIndexToID(scene->mNumMaterials);
		UUID prefabID = 0;
		LoadSourceMeta(loadedModel, loadedMaterials, loadedTextures,prefabID,m_registry.storage[id]);
		ExtractNodes(scene, scene->mRootNode, loadedModel,dirtyMeta);
		ExtractMaterialIDs(scene, loadedMaterials,materialIndexToID,dirtyMeta);
		ExtractTextureIDs(m_registry.storage[id].source, scene, loadedTextures, loadedMaterials,dirtyMeta);
		for (auto& [key, value] : loadedModel) {
			if (value.meshID == 0)
				continue;
			if (value.mesh.subMeshes.empty()) {
				ResourceManager::Get().UnloadResourceData(value.meshID);
				std::filesystem::path removedPath = (GetCurrentPath() / "Library" / "Client" / "Meshes" / value.meshID.ToString()).lexically_normal();
				removedPath += ".gmesh";
				if(std::filesystem::exists(removedPath))
					std::filesystem::remove(removedPath);
				continue;
			}
			int matIndex = 0;
			std::unordered_map<uint32_t, uint32_t> usedIndexes;
			for (auto& submesh : value.mesh.subMeshes) {
				auto it = usedIndexes.find(submesh.materialIndex);
				if (it != usedIndexes.end()) {
					submesh.materialIndex = it->second;
					continue;
				}
				value.materials.push_back(materialIndexToID.at(submesh.materialIndex));
				usedIndexes[submesh.materialIndex] = matIndex;
				submesh.materialIndex = matIndex++;
			}
			generated.push_back({ value.nodePath,value.id,AssetType::Mesh});
			std::filesystem::path cookedMeshPath = CookMesh(value.mesh, value.meshID);
			ResourceManager::Get().LoadResourceData(value.meshID, { cookedMeshPath,AssetType::Mesh }); // make LoadResourceData unload every resource of that type so it can refresh
		}
		for (auto& [key, value] : loadedTextures) {
			if (value.textureSlot == -10) {//expired / replaced texture
				if (m_registry.storage[value.id].isStandalone == true)
					continue;
				ResourceManager::Get().UnloadResourceData(value.id);
				std::filesystem::path removedPath = (GetCurrentPath() / "Library" / "Client" / "Textures" / value.id.ToString()).lexically_normal();
				removedPath += ".gtex";
				if (std::filesystem::exists(removedPath))
					std::filesystem::remove(removedPath);
				continue;
			}
			generated.push_back({ value.relativePath, value.id, AssetType::Texture });
			if (m_registry.Has(value.id))
			{
				uint64_t newContentHash;
				uint64_t finalHash;
				if (value.textureSlot != -1) {
					newContentHash = GetContentHash((void*)scene->mTextures[value.textureSlot]->pcData, scene->mTextures[value.textureSlot]->mWidth);
				}
				else {
					newContentHash = GetContentHash(m_registry.storage[value.id].source);
				}
				finalHash = GetContentHash(newContentHash, m_registry.storage[value.id].importSettings->GetHash());
				if (m_registry.storage[value.id].importHash == finalHash)
					continue;
				if (value.textureSlot != -1) //embedded texture
				{
					m_registry.storage[value.id].snapshotHash = SaveSnapshot(scene->mTextures[value.textureSlot]->pcData, scene->mTextures[value.textureSlot]->mWidth,
																m_registry.storage[value.id].snapshotHash, value.id.ToString());
					//modify in-memory metadata (not existent on disk)
					std::filesystem::path texturePath = (GetCurrentPath() / "Temp" / value.id.ToString()).lexically_normal();
					std::string fileExtension = scene->mTextures[value.textureSlot]->achFormatHint;
					if (fileExtension.empty())
						fileExtension = "png";
					texturePath += "." + fileExtension;
					std::filesystem::create_directories(texturePath.parent_path());
					std::ofstream file(texturePath, std::ios::binary);
					if (!file.is_open())
					{
						LOG_ERROR("[MODEL IMPORTER] FAILED TO OPEN FILE ${} ", texturePath);
						continue;
					}
					file.write(reinterpret_cast<const char*>(scene->mTextures[value.textureSlot]->pcData), scene->mTextures[value.textureSlot]->mWidth);
					file.close();
					MetaData& meta = m_registry.storage[value.id];
					meta.generatedFrom = id;
					meta.source = texturePath;
					TextureImportSettings* settings = static_cast<TextureImportSettings*>(meta.importSettings.get());
					meta.importHash = GetContentHash(stoull(key),meta.importSettings->GetHash());
					
					ImportTexture(meta.id, settings);
					if (std::filesystem::exists(texturePath))
						std::filesystem::remove(texturePath);
					continue;
				}
				// if its not embedded, it can't be non-standalone
			}
			else {
				bool isTemp = false;
				MetaData meta;
				meta.id = value.id;
				meta.importSettings = std::make_unique<TextureImportSettings>();
				meta.importHash = 0; //generate it on actual import
				meta.assetType = AssetType::Texture;
				if (value.textureSlot != -1) {
					std::string fileExtension = scene->mTextures[value.textureSlot]->achFormatHint;
					std::string filename = value.id.ToString();
					std::filesystem::path texturePath = (GetCurrentPath() / "Temp" / filename).lexically_normal();
					texturePath += "." + fileExtension;
					meta.source = texturePath;
					std::filesystem::create_directories(meta.source.parent_path());
					meta.snapshotHash = SaveSnapshot(scene->mTextures[value.textureSlot]->pcData, scene->mTextures[value.textureSlot]->mWidth,0,value.id.ToString());
					std::ofstream file(texturePath, std::ios::binary);
					if (!file.is_open())
					{
						LOG_ERROR("[MODEL IMPORTER] FAILED TO GENERATE FILE ${} ", texturePath);
						continue;
					}
					file.write(reinterpret_cast<const char*>(scene->mTextures[value.textureSlot]->pcData), scene->mTextures[value.textureSlot]->mWidth);
					file.close();
					meta.isStandalone = false;
					meta.generatedFrom = id;
					isTemp = true;
				}
				else {
					bool exists = false;
					std::filesystem::path texturePath = (m_registry.storage[id].source.parent_path() / value.relativePath).lexically_normal();
					std::filesystem::path stem = texturePath.stem();
					std::filesystem::path parentDir = texturePath.parent_path();
					if (texturePath.extension().string() == ".exr")
					{
						LOG_WARNING(".exr not supported yet, trying to find .png or .jpg");
						for (auto str : { ".png",".jpg",".jpeg" }) {
							std::filesystem::path file = (parentDir / stem).lexically_normal();
							file += str;
							if (std::filesystem::exists(file)){
								meta.source = file;
								exists = true;
								break;
							}
						}
					}
					else {
						for (auto str : { ".png",".jpg",".jpeg" }) {
							std::filesystem::path file = (parentDir / stem).lexically_normal();
							file += str;
							if (std::filesystem::exists(file)) {
								meta.source = file;
								exists = true;
								break;
							}
						}
					}
					if (!exists)
					{
						LOG_ERROR("TEXTURE FILE NOT FOUND , ACCOMPANYING FILES FOR THE MODEL MUST BE IN THE SAME FOLDER, ${}", texturePath);
						continue;
					}
					std::filesystem::path metaPath = texturePath.string() + ".meta";
					if (std::filesystem::exists(metaPath)) {
						YAML::Node metaFile;
						try {
							metaFile = YAML::LoadFile(metaPath.string());
						}
						catch (const YAML::Exception& e) {
							LOG_ERROR("${}  -file failed to open : ${} ", metaPath, e.what());
						}
						if (metaFile && metaFile["UUID"]) {
							UUID texID = metaFile["UUID"].as<uint64_t>();
							value.id = texID;
							continue;
						}
					}
					meta.isStandalone = true;
					meta.generatedFrom = 0;

				}
				if (!isTemp) {
					YAML::Node metafile;
					metafile["Source"] = meta.source.string();
					metafile["UUID"] = meta.id.Get();
					metafile["ImportHash"] = meta.importHash;
					metafile["Standalone"] = meta.isStandalone;
					metafile["ImportSettings"] = meta.importSettings->Serialize();
					metafile["Generated From"] = id.Get();
					metafile["Type"] = AssetTypeToString(meta.assetType);
					std::filesystem::path metapath = meta.source;
					metapath += ".meta";
					std::ofstream outputMetaFile(metapath);
					outputMetaFile << metafile;
					outputMetaFile.close();
				}
				m_registry.storage[meta.id] = meta;
				ImportTexture(meta.id, static_cast<TextureImportSettings*>(meta.importSettings.get()));
				if (isTemp == true) {
					if (std::filesystem::exists(meta.source))
						std::filesystem::remove(meta.source);
				}
			}
		}
		for (auto& [key, value] : loadedMaterials) {
			if (value.shaderType.empty()) {//expired / replaced mat
				if (m_registry.storage[value.id].isStandalone == true)
					continue;
				ResourceManager::Get().UnloadResourceData(value.id);
				std::filesystem::path removedPath = (GetCurrentPath() / "Library" / "Client" / "Materials" / value.id.ToString()).lexically_normal();
				removedPath += ".gmat";
				if (std::filesystem::exists(removedPath))
					std::filesystem::remove(removedPath);
				continue;
			}
			YAML::Node mat = BuildMaterialFile(scene, value);
			if (mat.IsNull() || !mat["Material"])
			{
				LOG_ERROR("[MODEL IMPORTER] failed to extract material ${}: ", value.name);
				continue;
			}
			generated.push_back({ key,value.id,AssetType::Material });
			if (m_registry.Has(value.id))
			{
				YAML::Emitter emitter;
				emitter << mat;
				if (m_registry.storage[value.id].importHash == GetContentHash((void*)emitter.c_str(), emitter.size()))
					continue;
				m_registry.storage[value.id].snapshotHash = SaveSnapshot((void*)emitter.c_str(), emitter.size(), m_registry.storage[value.id].snapshotHash, value.id.ToString());
				if (m_registry.storage[value.id].isStandalone == true) {
					continue;
				}
				else {
					std::filesystem::create_directories(m_registry.storage[value.id].source.parent_path());
					std::ofstream matfile(m_registry.storage[value.id].source);
					if (!matfile.is_open())
					{
						LOG_ERROR("FAILED TO CREATE TEMPORARY MATERIAL FILE ${}", m_registry.storage[value.id].source);
						continue;
					}
					matfile << mat;
					matfile.close();
					ImportMaterial(value.id);
					if(std::filesystem::exists(m_registry.storage[value.id].source))
						std::filesystem::remove(m_registry.storage[value.id].source);
				}
			}
			else {
				MetaData meta;
				meta.assetType = AssetType::Material;
				meta.generatedFrom = id;
				meta.id = value.id;
				meta.importHash = 0;
				meta.importSettings = nullptr;
				meta.isStandalone = false;
				meta.source = (GetCurrentPath() / "Temp" / meta.id.ToString()).lexically_normal();
				meta.source += ".gmat";
				std::filesystem::create_directories(meta.source.parent_path());
				std::ofstream matfile(meta.source);
				if (!matfile.is_open())
				{
					LOG_ERROR("FAILED TO CREATE TEMPORARY MATERIAL FILE ${}", meta.source);
					continue;
				}
				matfile << mat;
				matfile.close();
				YAML::Emitter emitter;
				emitter << mat;
				meta.snapshotHash = SaveSnapshot((void*)emitter.c_str(),emitter.size(),0,value.id.ToString());
				m_registry.storage[value.id] = meta;
				ImportMaterial(value.id);
				if(std::filesystem::exists(meta.source))
					std::filesystem::remove(meta.source);
			}
		}
		YAML::Node root;
		UUID rootUUID;
		std::unordered_map<UUID, YAML::Node> nodes;
		for (auto& [key, value] : loadedModel) {
			YAML::Node entity;
			YAML::Node node;
			node["UUID"] = value.id.Get();
			node["NodePath"] = value.nodePath.string();
			YAML::Node transform;
			aiVector3D scale;
			aiQuaternion rotation;
			aiVector3D position;
			value.transform.Decompose(scale, rotation, position);
			glm::quat glmQuat(rotation.w, rotation.x, rotation.y, rotation.z);
			glm::vec3 glmRot = glm::degrees(glm::eulerAngles(glmQuat));
			transform["Position"].push_back(std::vector<float>{position.x, position.y, position.z});
			transform["Rotation"].push_back(std::vector<float>{glmRot.x, glmRot.y, glmRot.z});
			transform["Scale"].push_back(std::vector<float>{scale.x, scale.y, scale.z});
			node["Transform"] = transform;
			if (value.meshID != 0) {
				YAML::Node meshRenderer;
				meshRenderer["Mesh"] = value.meshID.Get();
				for (auto& mat : value.materials)
					meshRenderer["Materials"].push_back(mat.Get());
				node["MeshRenderer"] = meshRenderer;
			}
			YAML::Node hierarchyMember;
			hierarchyMember["Parent"] = value.parent.Get();
			hierarchyMember["PrevSibling"] = 0;
			hierarchyMember["NextSibling"] = 0;
			hierarchyMember["FirstChild"] = 0;
			node["HierarchyMember"] = hierarchyMember;
			node["Children"] = YAML::Node(YAML::NodeType::Sequence);
			entity["Entity"] = node;
			nodes[value.id] = entity;
			if (value.parent == 0) {
				rootUUID = value.id;
			}
			
		}// build .gprefab
		for (auto& [key, value] : loadedModel) {
			if (value.parent == 0)
				continue;

			YAML::Node parent = nodes[value.parent]["Entity"];
			YAML::Node parentChildren = parent["Children"];
			YAML::Node child = nodes[value.id]["Entity"];

			if (parentChildren.size() == 0) {
				parent["HierarchyMember"]["FirstChild"] = value.id.Get();
			}
			else {
				parentChildren[parentChildren.size() - 1]["Entity"]["HierarchyMember"]["NextSibling"] = value.id.Get();
				child["HierarchyMember"]["PrevSibling"] =
					parentChildren[parentChildren.size() - 1]["Entity"]["UUID"].as<uint64_t>();
			}
			parent["Children"].push_back(nodes[value.id]);
		}
		root["Prefab"] = nodes[rootUUID];
		YAML::Emitter emitter;
		emitter << root;
		std::filesystem::path prefabPath = (m_registry.currentPath / id.ToString()).lexically_normal();
		prefabPath += ".gprefab";
		if (m_registry.Has(prefabID))
		{
			if (m_registry.storage[prefabID].importHash == GetContentHash((void*)emitter.c_str(), emitter.size()))
				return m_registry.storage[prefabID].source;
			m_registry.storage[prefabID].snapshotHash = SaveSnapshot((void*)emitter.c_str(), emitter.size(), m_registry.storage[prefabID].snapshotHash, prefabID.ToString());
			if (m_registry.storage[prefabID].isStandalone == true)
			{
				std::string metapath = m_registry.storage[prefabID].source.string() + ".meta";
				YAML::Node file = YAML::LoadFile(metapath);

				file["SnapshotHash"] = m_registry.storage[prefabID].snapshotHash;

				std::ofstream out(metapath);
				if (!out.is_open())
				{
					LOG_ERROR("PREFAB META FILE MODIFICATION FAILED ${}", m_registry.storage[prefabID].source);
					return std::filesystem::path();
				}

				out << file;
				out.close();
				return m_registry.storage[prefabID].source;
			}
		}
		std::ofstream prefabFile(prefabPath);
		if (!prefabFile.is_open())
		{
			LOG_ERROR("failed to create prefab file ${}", m_registry.storage[id].source);
			return std::filesystem::path();
		}
		prefabFile << root;
		prefabFile.close();
		if (prefabID == 0 || !m_registry.Has(prefabID))
			prefabID = UUID();
		MetaData meta;

		meta.source = prefabPath;
		meta.isStandalone = false;
		meta.generatedFrom = id;
		meta.id = prefabID;
		meta.importHash = 0;
		meta.assetType = AssetType::Prefab;
		if (m_registry.Has(prefabID)) 
			meta.snapshotHash = m_registry.storage[prefabID].snapshotHash;
		else
			meta.snapshotHash = SaveSnapshot((void*)emitter.c_str(), emitter.size(), 0, prefabID.ToString());
		YAML::Node metafile;
		metafile["Source"] = meta.source.string();
		metafile["UUID"] = meta.id.Get();
		metafile["ImportHash"] = meta.importHash;
		metafile["Standalone"] = meta.isStandalone;
		metafile["ImportSettings"] = 0;
		metafile["Generated From"] = meta.generatedFrom.Get();
		metafile["Type"] = AssetTypeToString(meta.assetType);
		metafile["SnapshotHash"] = meta.snapshotHash;
		std::filesystem::path metapath = prefabPath;
		metapath += ".meta";
		std::ofstream outputMetaFile(metapath);
		outputMetaFile << metafile;
		outputMetaFile.close();
		if(!m_registry.Has(prefabID))
			generated.push_back({ meta.source,meta.id,AssetType::Prefab });
		m_registry.storage[prefabID] = meta;
		if (dirtyMeta) {
			sourceMeta["Generated Dependencies"] = generated;
			std::ofstream sourceMetaFile(filepath.string() + ".meta");
			sourceMetaFile << sourceMeta;
			sourceMetaFile.close();
		}
		if (!ModifyMetaImportHash(m_registry.storage[id]))
			return std::filesystem::path();
		auto end = GetTime();
		LOG_INFO("Mesh loader took: ${} ", end - start);
		return ImportPrefab(prefabID);
	}
	std::filesystem::path AssetImporter::ImportTexture(const UUID& id,TextureImportSettings* settings)
	{
		TextureFileHeader header;
		std::filesystem::path filepath = m_registry.storage[id].source;
		std::string srcPath = filepath.string();
		if (!std::filesystem::exists(filepath) || !filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist",filepath);
			return std::filesystem::path();
		}
		int width, height, nrChannels;
		unsigned char* data = stbi_load(srcPath.c_str(), &width, &height, &nrChannels, 0);
		if (!data || !settings || settings->mipcount <= 0 )
		{
			LOG_ERROR("FAILED TO IMPORT : ${} ", srcPath);
			return std::filesystem::path();
		}
		std::vector<unsigned char> databuffer;
		uint32_t totalSize = width * height * nrChannels;
		int w = width, h = height;
		for (int i = 1; i < settings->mipcount; ++i) {
			w = (w >> 1) > 1 ? (w >> 1) : 1;
			h = (h >> 1) > 1 ? (h >> 1) : 1;
			totalSize += w * h * nrChannels;
		}
		databuffer.resize(totalSize);
		memcpy(databuffer.data(), data, width*height*nrChannels);
		STBI_FREE(data);
		std::vector<MipDataPacked> mips;
		mips.resize(settings->mipcount);
		header.width = width;
		header.height = height;
		mips[0] = { header.width,header.height,0, header.width * header.height * nrChannels };
		header.mipCount = settings->mipcount;
		memcpy(header.validation, "Texture",8);
		stbir_pixel_layout layout;
		switch (nrChannels)
		{
		case 1:
			header.format = PixelDataFormat::R8;
			layout = STBIR_1CHANNEL;
			break;
		case 2:
			header.format = PixelDataFormat::RG8;
			layout = STBIR_2CHANNEL;
			break;
		case 3:
			header.format = PixelDataFormat::RGB8;
			layout = STBIR_RGB;
			break;
		case 4:
			header.format = PixelDataFormat::RGBA8;
			layout = STBIR_RGBA;
			break;
		default:
			LOG_ERROR("INVALID NUMBER OF CHANNELS FOR ${}", srcPath);
			return std::filesystem::path();
			break;
		}
		for (int i = 1; i < header.mipCount; ++i) {
			int mipW = (width >> 1) > 1 ? (width >> 1) : 1;
			int mipH = (height >> 1) > 1 ? (height >> 1) : 1;
			mips[i].width = mipW;
			mips[i].height = mipH;
			mips[i].offset = mips[i - 1].offset + mips[i - 1].byteSize;
			mips[i].byteSize = mipW * mipH * nrChannels;
			stbir_resize_uint8_linear(databuffer.data() + mips[i-1].offset , width, height, 0,
				databuffer.data() + mips[i].offset , mipW, mipH, 0,
				layout);
			width = mipW;
			height = mipH;
		}
		header.dataSize = totalSize;
		std::string folder;
		if (id.GetFlag() == 0)
			folder = "Engine";
		else
			folder = "Client";
		std::filesystem::path outputPath = (GetCurrentPath() / "Library" / folder / "Textures" / id.ToString()).lexically_normal();
		outputPath += ".gtex";
		std::filesystem::create_directories(outputPath.parent_path());
		std::ofstream file(outputPath, std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to create file ${} ", outputPath);
			return std::filesystem::path();
		}
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		file.write(reinterpret_cast<const char*>(mips.data()), header.mipCount * sizeof(MipDataPacked));
		file.write(reinterpret_cast<const char*>(databuffer.data()), header.dataSize);
		file.close();
		if (!ModifyMetaImportHash(m_registry.storage[id]))
			return std::filesystem::path();
		ResourceManager::Get().LoadResourceData(id, { outputPath,AssetType::Texture });
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportShader(const UUID& id)
	{
		std::vector<std::pair<std::string, ShaderType>>parsedShaders;
		std::string currentShaderCode;
		ShaderType currentShaderType = ShaderType::None;
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!std::filesystem::exists(filepath) || !filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist",filepath);
			return std::filesystem::path();
		}
		std::stringstream shadersSource(DumpFileToString(filepath));
		std::string line;
		while (std::getline(shadersSource, line)) {
			std::stringstream identifierLine(line);
			std::string identifier;
			identifierLine >> identifier;
			if (identifier == "#type")
			{
				ShaderType newShaderType = ShaderType::None;
				std::string type;
				identifierLine >> type;
				if (type == "Vertex")
					newShaderType = ShaderType::Vertex;
				if (type == "Fragment")
					newShaderType = ShaderType::Fragment;

				if (newShaderType == ShaderType::None) {
					LOG_ERROR("INVALID SHADER TYPE, CHECK SHADER SOURCE CODE");
					return std::filesystem::path();
				}
				if (currentShaderType != ShaderType::None)
				{
					parsedShaders.push_back({ currentShaderCode,currentShaderType });
					currentShaderCode.clear();
				}
				currentShaderType = newShaderType;
			}
			else {
				currentShaderCode += line + "\n";
			}
		}
		if (currentShaderType != ShaderType::None) {
			parsedShaders.push_back({ currentShaderCode,currentShaderType });
		}
		std::string folder;
		if (id.GetFlag() == 0)
			folder = "Engine";
		else
			folder = "Client";
		std::filesystem::path outputPath = (GetCurrentPath() / "Library" / folder / "Shaders" / id.ToString()).lexically_normal();
		outputPath += ".gs";
		ShaderFileHeader header;
		header.shaderCount = parsedShaders.size();
		memcpy(&header.validation, "Shader",7);
		std::filesystem::create_directories(outputPath.parent_path());
		std::ofstream file(outputPath, std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to create file ${} ", outputPath);
			return std::filesystem::path();
		}
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		for (auto& [shader, type] : parsedShaders) {
			ShaderDataPacked data;
			data.size = shader.size();
			data.type = type;
			file.write(reinterpret_cast<const char*>(&data), sizeof(data));
			file.write(reinterpret_cast<const char*>(shader.data()), data.size);
		}
		file.close();
		if (!ModifyMetaImportHash(m_registry.storage[id]))
			return std::filesystem::path();
		ResourceManager::Get().LoadResourceData(id, { outputPath,AssetType::Shader });
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportMaterial(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!std::filesystem::exists(filepath) || !filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist", filepath);
			return std::filesystem::path();
		}
		std::string folder;
		if (id.GetFlag() == 0)
			folder = "Engine";
		else
			folder = "Client";
		std::filesystem::path outputPath = (GetCurrentPath() / "Library" / folder / "Materials" / id.ToString()).lexically_normal();
		outputPath += ".gmat";

		// check for missing textures / shaders
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return std::filesystem::path();
		}
		YAML::Node node = file["Material"];
		YAML::Node textures = node["Textures"];
		if (!node["Shader"] || !node["Tint"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return std::filesystem::path();
		}
		UUID shader = UUID(node["Shader"].as<uint64_t>(), true);
		if (!m_registry.Has(shader))
			LOG_ERROR("SHADER REFERENCED IN ${} NOT FOUND", filepath);

		std::filesystem::create_directories(outputPath.parent_path());
		std::filesystem::copy(filepath, outputPath,std::filesystem::copy_options::overwrite_existing);
		if (!ModifyMetaImportHash(m_registry.storage[id]))
			return std::filesystem::path();
		ResourceManager::Get().LoadResourceData(id, { outputPath,AssetType::Material });
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportPrefab(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!std::filesystem::exists(filepath) || !filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist", filepath);
			return std::filesystem::path();
		}
		std::string folder;
		if (id.GetFlag() == 0)
			folder = "Engine";
		else
			folder = "Client";
		std::filesystem::path outputPath = (GetCurrentPath() / "Library" / folder / "Prefabs" / id.ToString()).lexically_normal();
		outputPath += ".gprefab";

		// check for missing refs
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return std::filesystem::path();
		}
		YAML::Node entity = file["Prefab"];
		YAML::Node root = entity["Entity"];
		if (root["MeshRenderer"]) {
			YAML::Node meshRenderer = root["MeshRenderer"];
			UUID meshID = meshRenderer["Mesh"].as<uint64_t>();
			if (!ResourceManager::Get().IsResourceDataLoaded(meshID))
				LOG_WARNING("REGISTRY DOES NOT HAVE MESH  ${},", meshID.Get());
			for (auto node : meshRenderer["Materials"]) {
				UUID materialID = node.as<uint64_t>();
				if (!ResourceManager::Get().IsResourceDataLoaded(materialID))
					LOG_WARNING("REGISTRY DOES NOT HAVE MATERIAL ${},", materialID.Get());
			}
		}
		for (auto child : root["Children"]) {
			if (child["Entity"]["MeshRenderer"]) {
				YAML::Node meshRenderer = child["Entity"]["MeshRenderer"];
				UUID meshID = meshRenderer["Mesh"].as<uint64_t>();
				if (!ResourceManager::Get().IsResourceDataLoaded(meshID))
					LOG_WARNING("REGISTRY DOES NOT HAVE MESH  ${},", meshID.Get());
				for (auto node : meshRenderer["Materials"]) {
					UUID materialID = node.as<uint64_t>();
					if (!ResourceManager::Get().IsResourceDataLoaded(materialID))
						LOG_WARNING("REGISTRY DOES NOT HAVE MATERIAL ${},", materialID.Get());
				}
			}
		}
		std::filesystem::create_directories(outputPath.parent_path());
		std::filesystem::copy(filepath, outputPath, std::filesystem::copy_options::overwrite_existing);
		if (!ModifyMetaImportHash(m_registry.storage[id]))
			return std::filesystem::path();
		ResourceManager::Get().LoadResourceData(id, { outputPath,AssetType::Prefab });
		return outputPath;
	}
	uint64_t AssetImporter::GetContentHash(const std::filesystem::path& path) {
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR("Failed to open file for hashing: ${}", path);
			return 0;
		}
		XXH3_state_t* state = XXH3_createState();
		XXH3_64bits_reset(state);
		char buf[8192];
		while (file.read(buf, sizeof(buf)) || file.gcount() > 0) {
			XXH3_64bits_update(state, buf, static_cast<size_t>(file.gcount()));
		}
		uint64_t hash = XXH3_64bits_digest(state);
		XXH3_freeState(state);
		return hash;
		
	}
	uint64_t AssetImporter::GetContentHash(void* data, uint32_t size) {

		return XXH3_64bits(data, size);
	}
	uint64_t AssetImporter::GetContentHash(uint64_t firstHash, uint64_t secondHash) {
		uint64_t buf[2] = { firstHash,secondHash };
		return XXH3_64bits(buf, sizeof(buf));
	}
}