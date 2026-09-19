#include "pch.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/SerializationHelpers.h"
#include "Resources/ResourceManager.h"
#include "Render/Mesh.h"
#include "Core/Helpers.h"
#include <xxhash.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Gaze {
	namespace {
		const std::vector<aiTextureType> TextureTypes
		{ aiTextureType_BASE_COLOR,aiTextureType_DIFFUSE,aiTextureType_METALNESS,
			aiTextureType_DIFFUSE_ROUGHNESS,aiTextureType_NORMALS,aiTextureType_OPACITY,
			aiTextureType_SPECULAR,aiTextureType_SHININESS
		};

		constexpr const char* kShaderUnlit = "Unlit";
		constexpr const char* kShaderFlat = "Flat";
		constexpr const char* kShaderBlinn = "Blinn";
		constexpr const char* kShaderToon = "Toon";
		constexpr const char* kShaderPbr = "Pbr";

		// TextureNodeData::textureSlot
		//   >= 0 : index into scene->mTextures (embedded texture)
		//   -1   : external file that sits next to the model
		//   -10  : texture is no longer referenced by the model (expired)
		constexpr int kTextureSlotExternal = -1;
		constexpr int kTextureSlotExpired = -10;


		struct NodeData { 
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
			std::string shaderType; 
			std::unordered_map<aiTextureType, UUID> textures;
			int indexInMaterials = -1;

			bool IsExpired() const { return shaderType.empty(); }
		};

		struct TextureNodeData {
			UUID id;
			std::filesystem::path relativePath;
			int textureSlot = kTextureSlotExpired;

			bool IsExpired() const { return textureSlot == kTextureSlotExpired; }
			bool IsEmbedded() const { return textureSlot >= 0; }
		};

		// Everything we know about a model during one import: what the previous
		// import generated (loaded from .meta) merged with what the scene contains now.
		struct ModelContents {
			std::unordered_map<std::filesystem::path, NodeData> nodes;
			std::unordered_map<std::string, MaterialNodeData> materials;
			std::unordered_map<std::string, TextureNodeData> textures;
			std::vector<UUID> materialIndexToID; // scene material index -> material asset id
			UUID prefabID = 0;
			bool dirtyMeta = false;              // "Generated Dependencies" must be rewritten
		};

		struct ImporterCallbacks {
			std::function<std::filesystem::path(const UUID&, TextureImportSettings*)> importTexture;
			std::function<std::filesystem::path(const UUID&)> importMaterial;
			std::function<std::filesystem::path(const UUID&)> importPrefab;
		};

		//   /Library/<scope>/<assetDir>/<id><extension> (scope = Engine / Client)
		std::filesystem::path LibraryFile(const char* scope, const char* assetDir, const UUID& id, const char* extension) {
			std::filesystem::path path = (GetCurrentPath() / "Library" / scope / assetDir / id.ToString()).lexically_normal();
			path += extension;
			return path;
		}
		std::filesystem::path CookedFile(const UUID& id, const char* assetDir, const char* extension) {
			return LibraryFile(id.GetFlag() == 0 ? "Engine" : "Client", assetDir, id, extension);
		}


		// /Temp/<id><extension>  
		std::filesystem::path TempFile(const UUID& id, const std::string& extension) {
			std::filesystem::path path = (GetCurrentPath() / "Temp" / id.ToString()).lexically_normal();
			path += extension;
			return path;
		}

		void RemoveFileIfExists(const std::filesystem::path& path) {
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);
		}

		// Returns false only if the file could not be opened
		bool WriteYamlFile(const std::filesystem::path& path, const YAML::Node& node) {
			std::ofstream file(path);
			if (!file.is_open())
				return false;
			file << node;
			file.close();
			return true;
		}

		bool SourceFileExists(const std::filesystem::path& filepath) {
			if (!std::filesystem::exists(filepath) || !filepath.has_filename())
			{
				LOG_ERROR("${} Does not exist", filepath);
				return false;
			}
			return true;
		}

		bool OpenCookedFile(const std::filesystem::path& path, std::ofstream& file) {
			std::filesystem::create_directories(path.parent_path());
			file.open(path, std::ios::binary);
			if (!file.is_open())
			{
				LOG_ERROR("Failed to create file ${} ", path);
				return false;
			}
			return true;
		}

		void CopyIntoLibrary(const std::filesystem::path& source, const std::filesystem::path& destination) {
			std::filesystem::create_directories(destination.parent_path());
			std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing);
		}

		void DiscardCookedResource(const UUID& id, const char* assetDir, const char* extension) {
			ResourceManager::Get().ScheduleUnloadResourceData(id);
			RemoveFileIfExists(CookedFile(id ,assetDir, extension));
		}


		uint64_t HashEmitter(const YAML::Emitter& emitter) {
			return AssetImporter::GetContentHash((void*)emitter.c_str(), emitter.size());
		}

		uint64_t SaveSnapshot(void* snapshot, uint32_t size, uint64_t oldSnapshot, const std::string& name) {
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

		uint64_t SaveEmitterSnapshot(const YAML::Emitter& emitter, uint64_t oldSnapshot, const std::string& name) {
			return SaveSnapshot((void*)emitter.c_str(), emitter.size(), oldSnapshot, name);
		}

		bool ModifyMetaImportHash(MetaData& meta) {
			std::filesystem::path metaPath = meta.source;
			metaPath += ".meta";
			if (!std::filesystem::exists(metaPath) && meta.isStandalone == false)
				return true; // generated asset without a .meta on disk, nothing to update
			uint64_t hash = AssetImporter::GetContentHash(meta.source);
			if (meta.importSettings != nullptr)
				hash = AssetImporter::GetContentHash(hash, meta.importSettings->GetHash());
			meta.importHash = hash;

			YAML::Node metaFile;
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
			if (!WriteYamlFile(metaPath, metaFile))
			{
				LOG_ERROR("Failed to open meta file for writing: {}", metaPath);
				return false;
			}
			return true;
		}

		std::filesystem::path FinalizeImport(const UUID& id, MetaData& meta, const std::filesystem::path& outputPath, AssetType type) {
			if (!ModifyMetaImportHash(meta))
				return std::filesystem::path();
			ResourceManager::Get().ScheduleLoadResourceData(id, { outputPath, type });
			return outputPath;
		}

		void CollectPrefabNodeIDs(const YAML::Node& node, std::unordered_map<std::filesystem::path, NodeData>& out) {
			YAML::Node entity = node["Entity"];
			if (!entity || !entity["NodePath"] || !entity["UUID"])
				return;
			out[entity["NodePath"].as<std::string>()].id = entity["UUID"].as<uint64_t>();
			for (auto child : entity["Children"])
				CollectPrefabNodeIDs(child, out);
		}

		void CollectPrefabNodeIDsFromFile(const std::filesystem::path& filepath, std::unordered_map<std::filesystem::path, NodeData>& out) {
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
			CollectPrefabNodeIDs(prefab, out);
		}


		void LoadPreviousImportState(const MetaData& sourceMeta, ModelContents& model) {
			for (auto& dependency : sourceMeta.generatedDependencies) {
				const std::string nodePathStr = dependency.nodePath.string();
				switch (dependency.type) {
				case AssetType::Material: {
					MaterialNodeData material;
					material.id = dependency.id;
					material.name = nodePathStr;
					model.materials[material.name] = material;
					break;
				}
				case AssetType::Texture:
					model.textures[nodePathStr].id = dependency.id;
					break;
				case AssetType::Mesh: {
					NodeData mesh;
					mesh.meshID = dependency.id;
					mesh.nodePath = dependency.nodePath;
					model.nodes[dependency.nodePath] = mesh;
					break;
				}
				case AssetType::Prefab:
					model.prefabID = dependency.id;
					CollectPrefabNodeIDsFromFile(dependency.nodePath, model.nodes); // for a prefab nodePath is just its sourcePath
					break;
				default:
					LOG_ERROR("[MODEL IMPORTER] ASSET TYPE UNKNOWN, REGENERATING ASSET");
					continue;
				}
			}
		}


		std::filesystem::path GetNodePath(const aiNode* node) {
			if (node == nullptr)
				return {};
			if (node->mParent == nullptr)
				return node->mName.C_Str();
			return GetNodePath(node->mParent) / node->mName.C_Str();
		}

		UUID ResolveParentID(const aiNode* node, std::unordered_map<std::filesystem::path, NodeData>& nodes) {
			UUID parent = 0;
			const std::filesystem::path parentNodePath = GetNodePath(node->mParent);
			if (!parentNodePath.empty())
				parent = nodes[parentNodePath].id; // guaranteed to be generated (parents are visited first)
			return parent;
		}

		void AppendSubMesh(MeshData& out, const aiMesh* mesh, bool bufferHasUVs, uint32_t& indexOffset, uint32_t& vertexOffset) {
			auto& pos = mesh->mVertices;
			auto& normals = mesh->mNormals;
			auto& textureCoords = mesh->mTextureCoords;
			const bool meshHasUVs = mesh->HasTextureCoords(0);
			for (uint32_t index = 0; index < mesh->mNumVertices; ++index) {
				out.bufferData[0].data.push_back(pos[index].x);
				out.bufferData[0].data.push_back(pos[index].y);
				out.bufferData[0].data.push_back(pos[index].z);
				out.bufferData[0].data.push_back(normals[index].x);
				out.bufferData[0].data.push_back(normals[index].y);
				out.bufferData[0].data.push_back(normals[index].z);
				if (meshHasUVs) {
					out.bufferData[0].data.push_back(textureCoords[0][index].x);
					out.bufferData[0].data.push_back(textureCoords[0][index].y);
				}
				else if (bufferHasUVs) { // other submeshes have UVs, keep the vertex layout uniform
					out.bufferData[0].data.emplace_back();
					out.bufferData[0].data.emplace_back();
				}
			}
			auto& faces = mesh->mFaces;
			uint32_t indexCount = 0;
			for (uint32_t index = 0; index < mesh->mNumFaces; ++index) {
				auto& face = faces[index];
				for (uint32_t index2 = 0; index2 < face.mNumIndices; ++index2)
				{
					out.indices.push_back(face.mIndices[index2] + vertexOffset); //assimp face indices local to current mesh
					++indexCount;
				}
			}
			out.subMeshes.push_back({ indexCount,indexOffset,mesh->mMaterialIndex });
			LOG_INFO("subMesh offset: ${} subMesh Count: ${} ", indexOffset, indexCount);
			indexOffset += indexCount;
			vertexOffset += mesh->mNumVertices;
		}

		MeshData BuildMeshData(const aiScene* scene, const aiNode* node) {
			MeshData data;
			data.bufferData[0].layout.Add(AttributeDataType::Float3); // pos mandatory
			data.bufferData[0].layout.Add(AttributeDataType::Float3); //normals generated by assimp if not existent
			bool hasUVs = false;
			for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
				if (scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(0))
					hasUVs = true;
			}
			if (hasUVs)
				data.bufferData[0].layout.Add(AttributeDataType::Float2);

			uint32_t indexOffset{};
			uint32_t vertexOffset{}; // for indices
			for (uint32_t i = 0; i < node->mNumMeshes; ++i) //submesh
				AppendSubMesh(data, scene->mMeshes[node->mMeshes[i]], hasUVs, indexOffset, vertexOffset);
			return data;
		}

		// Walks the assimp node tree and creates/updates a NodeData for every node.
		void ExtractNodes(const aiScene* scene, const aiNode* node, ModelContents& model) {
			auto& nodes = model.nodes;
			const std::filesystem::path nodePath = GetNodePath(node);

			NodeData loaded;
			bool extractMesh; // known nodes always go through mesh extraction, so stale meshes can be detected
			auto known = nodes.find(nodePath);
			if (known != nodes.end()) {
				loaded = known->second;
				if (loaded.id == 0)
					loaded.id = UUID();
				extractMesh = true;
			}
			else {
				loaded.id = UUID();
				if (node->mNumMeshes != 0) {
					loaded.meshID = UUID();
					model.dirtyMeta = true;
					extractMesh = true;
				}
				else {
					loaded.meshID = 0;
					extractMesh = false;
				}
			}
			loaded.nodePath = nodePath;
			if (extractMesh)
				loaded.mesh = BuildMeshData(scene, node);
			loaded.transform = node->mTransformation;
			loaded.parent = ResolveParentID(node, nodes);
			nodes[nodePath] = loaded;

			for (uint32_t index = 0; index < node->mNumChildren; ++index)
				ExtractNodes(scene, node->mChildren[index], model);
		}

		void RemapSubMeshMaterials(NodeData& node, const std::vector<UUID>& materialIndexToID) {
			int matIndex = 0;
			std::unordered_map<uint32_t, uint32_t> usedIndexes;
			for (auto& submesh : node.mesh.subMeshes) {
				auto it = usedIndexes.find(submesh.materialIndex);
				if (it != usedIndexes.end()) {
					submesh.materialIndex = it->second;
					continue;
				}
				node.materials.push_back(materialIndexToID.at(submesh.materialIndex));
				usedIndexes[submesh.materialIndex] = matIndex;
				submesh.materialIndex = matIndex++;
			}
		}

		// Writes the .gmesh binary
		std::filesystem::path CookMesh(const MeshData& mesh, const UUID& uuid) {
			std::filesystem::path filepath = CookedFile(uuid,"Meshes", ".gmesh");
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
				int index1 = 0;
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

		std::string GetMaterialName(aiMaterial* mat, uint32_t index) {
			std::string name = mat->GetName().C_Str();
			if (name.empty())
				name = "Material_" + std::to_string(index);
			return name;
		}

		std::string ShaderTypeForShadingMode(uint32_t shading) {
			switch (shading) {
			case aiShadingMode::aiShadingMode_NoShading:
				return kShaderUnlit;

			case aiShadingMode::aiShadingMode_Flat:
				return kShaderFlat;

			case aiShadingMode::aiShadingMode_Gouraud:
			case aiShadingMode::aiShadingMode_Phong:
			case aiShadingMode::aiShadingMode_Blinn:
				return kShaderBlinn;

			case aiShadingMode::aiShadingMode_Toon:
				return kShaderToon;

			case aiShadingMode::aiShadingMode_OrenNayar:
			case aiShadingMode::aiShadingMode_Minnaert:
			case aiShadingMode::aiShadingMode_CookTorrance:
			case aiShadingMode::aiShadingMode_Fresnel:
			case aiShadingMode::aiShadingMode_PBR_BRDF:
				return kShaderPbr;

			default:
				return kShaderPbr;
			}
		}

		// Assigns an asset id + shader type to every scene material
		void ExtractMaterials(const aiScene* scene, ModelContents& model) {
			std::unordered_map<std::filesystem::path, uint32_t> nameCount;
			for (uint32_t index = 0; index < scene->mNumMaterials; ++index) {
				aiMaterial* mat = scene->mMaterials[index];
				const std::string materialName = GetMaterialName(mat, index);
				nameCount[materialName]++;
				if (model.materials.find(materialName) == model.materials.end())
				{
					model.materials[materialName].name = materialName;
					model.materials[materialName].id = UUID();
					model.dirtyMeta = true;
				}
				else if (nameCount[materialName] > 1)
				{
					LOG_ERROR("DUPLICATE MATERIAL NAME FOUND IN FILE");
					continue;
				}
				uint32_t shading = aiShadingMode_PBR_BRDF;
				model.materials[materialName].indexInMaterials = index;
				mat->Get(AI_MATKEY_SHADING_MODEL, shading);
				model.materials[materialName].shaderType = ShaderTypeForShadingMode(shading);
				model.materialIndexToID[index] = model.materials[materialName].id;
			}
		}

		void WriteMaterialTint(YAML::Node& materialNode, aiMaterial* mat) {
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
		}

		void WriteShaderSpecificParams(YAML::Node& materialNode, aiMaterial* mat, const std::string& shaderType) {
			if (shaderType == kShaderPbr) {
				float metallic = 0.0f, roughness = 1.0f;
				mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
				mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
				materialNode["Metallic"] = metallic;
				materialNode["Roughness"] = roughness;
			}
			else if (shaderType == kShaderBlinn) {
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
		}

		void WriteTextureSlots(YAML::Node& materialNode, const std::unordered_map<aiTextureType, UUID>& textures) {
			YAML::Node tex;
			tex["Albedo"] = ReservedUUID::DEFAULTTEXTURE.Get();
			for (auto& [slot, texture] : textures) {
				std::string slotString = aiTextureTypeToString(slot);
				if (slotString == "Diffuse" || slotString == "BaseColor")
					slotString = "Albedo";
				tex[slotString] = texture.Get();
			}
			materialNode["Textures"] = tex;
		}

		// Builds the YAML of a .gmat file. Returns an empty node on failure.
		YAML::Node BuildMaterialFile(const aiScene* scene, const MaterialNodeData& data) {
			YAML::Node root;
			YAML::Node materialNode;

			if (data.indexInMaterials < 0 || static_cast<uint32_t>(data.indexInMaterials) >= scene->mNumMaterials) {
				LOG_ERROR("[MATERIAL BUILDER] material '${}' not found in scene", data.name);
				return root;
			}
			aiMaterial* mat = scene->mMaterials[data.indexInMaterials];
			materialNode["Shader"] = ReservedUUID::DEFAULTSHADER.Get();
			WriteMaterialTint(materialNode, mat);
			WriteShaderSpecificParams(materialNode, mat, data.shaderType);
			WriteTextureSlots(materialNode, data.textures);

			root["Material"] = materialNode;
			return root;
		}

		// Creates/overwrites the temporary .gmat source that ImportMaterial reads
		bool WriteTempMaterialFile(const std::filesystem::path& path, const YAML::Node& mat) {
			std::filesystem::create_directories(path.parent_path());
			if (!WriteYamlFile(path, mat))
			{
				LOG_ERROR("FAILED TO CREATE TEMPORARY MATERIAL FILE ${}", path);
				return false;
			}
			return true;
		}
		
		bool FindSupportedTextureFile(const std::filesystem::path& texturePath, std::filesystem::path& out) {
			if (texturePath.extension().string() == ".exr")
				LOG_WARNING(".exr not supported yet, trying to find .png or .jpg");
			const std::filesystem::path stem = texturePath.stem();
			const std::filesystem::path parentDir = texturePath.parent_path();
			for (auto extension : { ".png",".jpg",".jpeg" }) {
				std::filesystem::path file = (parentDir / stem).lexically_normal();
				file += extension;
				if (std::filesystem::exists(file)) {
					out = file;
					return true;
				}
			}
			return false;
		}


		const aiTexture& EmbeddedTextureAt(const aiScene* scene, int slot) {
			return *scene->mTextures[slot];
		}

		uint64_t HashEmbeddedTexture(const aiTexture& texture) {
			return AssetImporter::GetContentHash((void*)texture.pcData, texture.mWidth);
		}

		uint64_t SaveEmbeddedTextureSnapshot(const aiTexture& texture, uint64_t oldSnapshot, const std::string& name) {
			return SaveSnapshot((void*)texture.pcData, texture.mWidth, oldSnapshot, name);
		}

		std::string EmbeddedTextureExtension(const aiTexture& texture, const char* fallback) {
			std::string extension = texture.achFormatHint;
			if (extension.empty())
				extension = fallback;
			return extension;
		}

		// Dumps the compressed embedded data to a file so the regular texture importer can read it
		bool WriteEmbeddedTexture(const aiTexture& texture, const std::filesystem::path& path) {
			std::filesystem::create_directories(path.parent_path());
			std::ofstream file(path, std::ios::binary);
			if (!file.is_open())
			{
				LOG_ERROR("[MODEL IMPORTER] FAILED TO WRITE EMBEDDED TEXTURE ${} ", path);
				return false;
			}
			file.write(reinterpret_cast<const char*>(texture.pcData), texture.mWidth);
			file.close();
			return true;
		}

		bool RegisterEmbeddedTexture(const aiScene* scene, const char* sourceStr, ModelContents& model, UUID& outID) {
			uint32_t textureSlot = std::atoi(sourceStr + 1);
			const aiTexture& embedded = EmbeddedTextureAt(scene, textureSlot);
			if (embedded.mHeight != 0)
			{
				LOG_ERROR("Embedded texture must be compressed ,uncompressed format not supported ");
				return false;
			}
			const std::string key = std::to_string(HashEmbeddedTexture(embedded));
			if (model.textures.find(key) == model.textures.end()) {
				model.textures[key].id = UUID();
				model.dirtyMeta = true;
			}
			model.textures[key].relativePath = key;
			model.textures[key].textureSlot = textureSlot;
			outID = model.textures[key].id;
			return true;
		}

		UUID RegisterExternalTexture(const std::filesystem::path& modelSource, const char* sourceStr, ModelContents& model) {
			std::filesystem::path path{ sourceStr };
			if (!std::filesystem::exists(modelSource.parent_path() / path.parent_path()))
				path = path.filename();
			path = path.lexically_normal();
			std::string pathStr = path.generic_string();
			if (model.textures.find(pathStr) == model.textures.end()) {
				model.textures[pathStr].id = UUID();
				model.dirtyMeta = true;
			}
			const std::filesystem::path texturePath = modelSource.parent_path() / path;
			std::filesystem::path supportedTextureFile;
			if (!FindSupportedTextureFile(texturePath, supportedTextureFile))
			{
				LOG_ERROR("TEXTURE FILE NOT FOUND , ACCOMPANYING FILES FOR THE MODEL MUST BE IN THE SAME FOLDER, ${}", texturePath);
				return ReservedUUID::NONE;
			}

			const std::filesystem::path metaPath = supportedTextureFile.string() + ".meta";
			if (std::filesystem::exists(metaPath)) {
				YAML::Node metaFile;
				try {
					metaFile = YAML::LoadFile(metaPath.string());
				}
				catch (const YAML::Exception& e) {
					LOG_ERROR("${}  -file failed to open : ${} ", metaPath, e.what());
				}
				if (metaFile && metaFile["UUID"]) {
					UUID existingID = metaFile["UUID"].as<uint64_t>();
					model.textures[pathStr].id = existingID;
				}
			}
			model.textures[pathStr].relativePath = pathStr;
			model.textures[pathStr].textureSlot = kTextureSlotExternal;
			return model.textures[pathStr].id;
		}

		void ExtractTextures(const std::filesystem::path& modelSource, const aiScene* scene, ModelContents& model) {
			for (uint32_t index = 0; index < scene->mNumMaterials; ++index) {
				aiMaterial* mat = scene->mMaterials[index];
				const std::string materialName = GetMaterialName(mat, index);
				for (auto& slot : TextureTypes) {
					aiString source;
					if (mat->GetTexture(slot, 0, &source) != aiReturn_SUCCESS)
						continue;

					const char* sourceStr = source.C_Str();
					UUID textureID = 0;
					if (sourceStr[0] == '*') {
						if (!RegisterEmbeddedTexture(scene, sourceStr, model, textureID))
							continue;
					}
					else {
						textureID = RegisterExternalTexture(modelSource, sourceStr, model);
					}
					model.materials[materialName].textures[slot] = textureID;
				}
			}
		}


		void WriteTextureMetaFile(MetaData& meta, const UUID& modelID) {
			YAML::Node metafile;
			metafile["Source"] = meta.source.string();
			metafile["UUID"] = meta.id.Get();
			metafile["ImportHash"] = meta.importHash;
			metafile["SnapshotHash"] = meta.snapshotHash;
			metafile["Standalone"] = meta.isStandalone;
			metafile["ImportSettings"] = meta.importSettings->Serialize();
			metafile["Generated From"] = modelID.Get();
			metafile["Type"] = AssetTypeToString(meta.assetType);
			std::filesystem::path metapath = meta.source;
			metapath += ".meta";
			WriteYamlFile(metapath, metafile);
		}

		YAML::Node BuildTransformNode(aiMatrix4x4 matrix) {
			YAML::Node transform;
			aiVector3D scale;
			aiQuaternion rotation;
			aiVector3D position;
			matrix.Decompose(scale, rotation, position);
			glm::quat glmQuat(rotation.w, rotation.x, rotation.y, rotation.z);
			glm::vec3 glmRot = glm::degrees(glm::eulerAngles(glmQuat));
			transform["Position"] = std::vector<float>{ position.x, position.y, position.z };
			transform["Rotation"] = std::vector<float>{ glmRot.x, glmRot.y, glmRot.z };
			transform["Scale"] = std::vector<float>{ scale.x, scale.y, scale.z };
			return transform;
		}

		// { Entity: {...} } for one node, hierarchy links are filled in later
		YAML::Node BuildEntityNode(const NodeData& value) {
			YAML::Node entity;
			YAML::Node node;
			node["UUID"] = value.id.Get();
			node["NodePath"] = value.nodePath.string();
			node["Transform"] = BuildTransformNode(value.transform);
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
			return entity;
		}

		void LinkPrefabHierarchy(const std::unordered_map<std::filesystem::path, NodeData>& model, std::unordered_map<UUID, YAML::Node>& nodes) {
			for (auto& [key, value] : model) {
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
		}

		YAML::Node BuildPrefabYaml(const std::unordered_map<std::filesystem::path, NodeData>& model) {
			YAML::Node root;
			UUID rootUUID;
			std::unordered_map<UUID, YAML::Node> nodes;
			for (auto& [key, value] : model) {
				nodes[value.id] = BuildEntityNode(value);
				if (value.parent == 0)
					rootUUID = value.id;
			}
			LinkPrefabHierarchy(model, nodes);
			root["Prefab"] = nodes[rootUUID];
			return root;
		}

		void WritePrefabMetaFile(const MetaData& meta) {
			YAML::Node metafile;
			metafile["Source"] = meta.source.string();
			metafile["UUID"] = meta.id.Get();
			metafile["ImportHash"] = meta.importHash;
			metafile["Standalone"] = meta.isStandalone;
			metafile["ImportSettings"] = 0;
			metafile["Generated From"] = meta.generatedFrom.Get();
			metafile["Type"] = AssetTypeToString(meta.assetType);
			metafile["SnapshotHash"] = meta.snapshotHash;
			std::filesystem::path metapath = meta.source;
			metapath += ".meta";
			WriteYamlFile(metapath, metafile);
		}

		void WarnAboutMissingResources(YAML::Node entity) {
			if (!entity["MeshRenderer"])
				return;
			YAML::Node meshRenderer = entity["MeshRenderer"];
			UUID meshID = meshRenderer["Mesh"].as<uint64_t>();
			if (!ResourceManager::Get().IsResourceDataLoaded(meshID))
				LOG_WARNING("REGISTRY DOES NOT HAVE MESH  ${},", meshID.Get());
			for (auto node : meshRenderer["Materials"]) {
				UUID materialID = node.as<uint64_t>();
				if (!ResourceManager::Get().IsResourceDataLoaded(materialID))
					LOG_WARNING("REGISTRY DOES NOT HAVE MATERIAL ${},", materialID.Get());
			}
		}

		template <typename Registry>
		class ModelImportSession {
		public:
			ModelImportSession(Registry& registry, const UUID& modelID, ImporterCallbacks callbacks)
				: m_registry(registry), m_modelID(modelID), m_callbacks(std::move(callbacks)) {
			}

			std::filesystem::path Run()
			{
				m_sourcePath = m_registry.storage[m_modelID].source;
				m_sourceMetaPath = m_sourcePath;
				m_sourceMetaPath += ".meta";

				if (!LoadSourceMetaFile())
					return std::filesystem::path();
				if (!ReadScene())
					return std::filesystem::path();

				ExtractModelContents();
				ProcessMeshes();
				ProcessTextures();
				ProcessMaterials();
				return FinishPrefab();
			}

		private:

			bool LoadSourceMetaFile()
			{
				try {
					m_sourceMeta = YAML::LoadFile(m_sourceMetaPath.string());
				}
				catch (const YAML::Exception& e) {
					LOG_ERROR("${}  -file failed to open : ${} ", m_sourcePath, e.what());
					return false;
				}
				return true;
			}

			bool ReadScene()
			{
				m_scene = m_importer.ReadFile(m_sourcePath.string(),
					aiProcess_Triangulate |
					aiProcess_GenNormals |
					aiProcess_GenUVCoords);
				if (m_scene == nullptr || m_scene->mNumMeshes == 0) {
					LOG_ERROR("Failed to import model ${}", m_sourcePath);
					return false;
				}
				return true;
			}

			// Merge what the last import generate" with what the scene contains now
			void ExtractModelContents()
			{
				m_model.materialIndexToID = std::vector<UUID>(m_scene->mNumMaterials);
				LoadPreviousImportState(m_registry.storage[m_modelID], m_model);
				if (m_model.prefabID == 0)
					m_model.prefabID = UUID();
				ExtractNodes(m_scene, m_scene->mRootNode, m_model);
				ExtractMaterials(m_scene, m_model);
				ExtractTextures(m_sourcePath, m_scene, m_model);
			}

			void ProcessMeshes()
			{
				for (auto& entry : m_model.nodes) {
					NodeData& node = entry.second;
					if (node.meshID == 0)
						continue;
					if (node.mesh.subMeshes.empty()) { // node lost its meshes
						DiscardCookedResource(node.meshID, "Meshes", ".gmesh");
						m_model.dirtyMeta = true;
						continue;
					}
					RemapSubMeshMaterials(node, m_model.materialIndexToID);
					m_generated.push_back({ node.nodePath,node.meshID,AssetType::Mesh });
					std::filesystem::path cookedMeshPath = CookMesh(node.mesh, node.meshID);
					ResourceManager::Get().ScheduleLoadResourceData(node.meshID, { cookedMeshPath,AssetType::Mesh }); 
				}
			}

			void ProcessTextures()
			{
				for (auto& entry : m_model.textures) {
					const std::string& key = entry.first;
					TextureNodeData& texture = entry.second;
					if (texture.IsExpired()) {
						DiscardExpiredTexture(texture);
						continue;
					}
					m_generated.push_back({ texture.relativePath, texture.id, AssetType::Texture });
					if (m_registry.Has(texture.id))
						RefreshKnownTexture(key, texture);
					else
						ImportNewTexture(texture);
				}
			}

			void DiscardExpiredTexture(const TextureNodeData& texture)
			{
				m_model.dirtyMeta = true;
				if (m_registry.storage[texture.id].isStandalone == true)
					return; 
				DiscardCookedResource(texture.id, "Textures", ".gtex");
			}

			void RefreshKnownTexture(const std::string& key, TextureNodeData& texture)
			{
				MetaData& meta = m_registry.storage[texture.id];
				const bool embedded = texture.IsEmbedded();

				uint64_t newContentHash;
				if (embedded)
					newContentHash = HashEmbeddedTexture(EmbeddedTextureAt(m_scene, texture.textureSlot));
				else
					newContentHash = AssetImporter::GetContentHash(meta.source);
				const uint64_t finalHash = AssetImporter::GetContentHash(newContentHash, meta.importSettings->GetHash());
				if (meta.importHash == finalHash)
					return; 

				// a changed *external* texture is not reimported here, nothing happens.
				if (!embedded)
					return;

				const aiTexture& embeddedTexture = EmbeddedTextureAt(m_scene, texture.textureSlot);
				meta.snapshotHash = SaveEmbeddedTextureSnapshot(embeddedTexture, meta.snapshotHash, texture.id.ToString());

				//modify in-memory metadata (not existent on disk)
				const std::filesystem::path texturePath = TempFile(texture.id, "." + EmbeddedTextureExtension(embeddedTexture, "png"));
				if (!WriteEmbeddedTexture(embeddedTexture, texturePath))
					return;
				meta.generatedFrom = m_modelID;
				meta.source = texturePath; 
				TextureImportSettings* settings = static_cast<TextureImportSettings*>(meta.importSettings.get());
				meta.importHash = AssetImporter::GetContentHash(std::stoull(key), meta.importSettings->GetHash());

				m_callbacks.importTexture(meta.id, settings);
				RemoveFileIfExists(texturePath);
			}

			// Texture not in the registry yet: create its MetaData and import it
			void ImportNewTexture(TextureNodeData& texture)
			{
				MetaData meta;
				meta.id = texture.id;
				meta.importSettings = std::make_unique<TextureImportSettings>();
				meta.importHash = 0; //generate it on actual import
				meta.assetType = AssetType::Texture;
				meta.snapshotHash = 0;

				bool isTemp = false;
				if (texture.IsEmbedded()) {
					if (!PrepareEmbeddedTexture(texture, meta))
						return;
					isTemp = true;
				}
				else {
					if (!PrepareExternalTexture(texture, meta))
						return;
				}

				if (!isTemp)
					WriteTextureMetaFile(meta, m_modelID);
				m_registry.storage[meta.id] = meta;
				m_callbacks.importTexture(meta.id, static_cast<TextureImportSettings*>(meta.importSettings.get()));
				if (isTemp)
					RemoveFileIfExists(meta.source);
			}

			bool PrepareEmbeddedTexture(const TextureNodeData& texture, MetaData& meta)
			{
				const aiTexture& embedded = EmbeddedTextureAt(m_scene, texture.textureSlot);
				meta.source = TempFile(texture.id, "." + EmbeddedTextureExtension(embedded, "png"));
				meta.snapshotHash = SaveEmbeddedTextureSnapshot(embedded, 0, texture.id.ToString());
				if (!WriteEmbeddedTexture(embedded, meta.source))
					return false;
				meta.isStandalone = false;
				meta.generatedFrom = m_modelID;
				return true;
			}

			bool PrepareExternalTexture(TextureNodeData& texture, MetaData& meta)
			{
				const std::filesystem::path texturePath = (m_sourcePath.parent_path() / texture.relativePath).lexically_normal();
				if (!FindSupportedTextureFile(texturePath, meta.source))
				{
					LOG_ERROR("TEXTURE FILE NOT FOUND , ACCOMPANYING FILES FOR THE MODEL MUST BE IN THE SAME FOLDER, ${}", texturePath);
					return false;
				}
				meta.isStandalone = true;
				meta.generatedFrom = 0;
				return true;
			}

			void ProcessMaterials()
			{
				for (auto& entry : m_model.materials) {
					const std::string& key = entry.first;
					MaterialNodeData& material = entry.second;
					if (material.IsExpired()) {
						DiscardExpiredMaterial(material);
						continue;
					}
					YAML::Node mat = BuildMaterialFile(m_scene, material);
					if (mat.IsNull() || !mat["Material"])
					{
						LOG_ERROR("[MODEL IMPORTER] failed to extract material ${}: ", material.name);
						continue;
					}
					m_generated.push_back({ key,material.id,AssetType::Material });
					if (m_registry.Has(material.id))
						RefreshKnownMaterial(mat, material);
					else
						ImportNewMaterial(mat, material);
				}
			}

			void DiscardExpiredMaterial(const MaterialNodeData& material)
			{
				m_model.dirtyMeta = true;
				if (m_registry.storage[material.id].isStandalone == true)
					return;
				DiscardCookedResource(material.id, "Materials", ".gmat");
			}

			void RefreshKnownMaterial(const YAML::Node& mat, const MaterialNodeData& material)
			{
				MetaData& meta = m_registry.storage[material.id];
				YAML::Emitter emitter;
				emitter << mat;
				if (meta.importHash == HashEmitter(emitter))
					return; // unchanged
				meta.snapshotHash = SaveEmitterSnapshot(emitter, meta.snapshotHash, material.id.ToString());
				if (meta.isStandalone == true)
					return; // user-owned material, only the snapshot is updated
				if (!WriteTempMaterialFile(meta.source, mat))
					return;
				m_callbacks.importMaterial(material.id);
				RemoveFileIfExists(meta.source);
			}

			void ImportNewMaterial(const YAML::Node& mat, const MaterialNodeData& material)
			{
				MetaData meta;
				meta.assetType = AssetType::Material;
				meta.generatedFrom = m_modelID;
				meta.id = material.id;
				meta.importHash = 0;
				meta.importSettings = nullptr;
				meta.isStandalone = false;
				meta.source = TempFile(meta.id, ".gmat");
				if (!WriteTempMaterialFile(meta.source, mat))
					return;
				YAML::Emitter emitter;
				emitter << mat;
				meta.snapshotHash = SaveEmitterSnapshot(emitter, 0, material.id.ToString());
				m_registry.storage[material.id] = meta;
				m_callbacks.importMaterial(material.id);
				RemoveFileIfExists(meta.source);
			}


			std::filesystem::path FinishPrefab()
			{
				YAML::Node prefabYaml = BuildPrefabYaml(m_model.nodes);
				YAML::Emitter emitter;
				emitter << prefabYaml;

				if (!ModifyMetaImportHash(m_registry.storage[m_modelID]))
					return std::filesystem::path();

				std::filesystem::path earlyResult;
				if (TryUpdateExistingPrefab(emitter, earlyResult))
					return earlyResult;

				if (!WritePrefab(prefabYaml, emitter))
					return std::filesystem::path();
				PersistSourceMeta();
				
				return m_callbacks.importPrefab(m_model.prefabID);
			}

			bool TryUpdateExistingPrefab(const YAML::Emitter& emitter, std::filesystem::path& result)
			{
				const UUID& prefabID = m_model.prefabID;
				if (!m_registry.Has(prefabID))
					return false;

				MetaData& existing = m_registry.storage[prefabID];
				if (existing.importHash == HashEmitter(emitter)) {
					result = existing.source;
					return true;
				}
				existing.snapshotHash = SaveEmitterSnapshot(emitter, existing.snapshotHash, prefabID.ToString());
				if (existing.isStandalone != true)
					return false;

				std::string metapath = existing.source.string() + ".meta";
				YAML::Node file = YAML::LoadFile(metapath); // PRESERVED QUIRK: not guarded, throws if the .meta is missing/invalid
				file["SnapshotHash"] = existing.snapshotHash;
				if (!WriteYamlFile(metapath, file))
				{
					LOG_ERROR("PREFAB META FILE MODIFICATION FAILED ${}", existing.source);
					result = std::filesystem::path();
					return true;
				}
				result = existing.source;
				return true;
			}

			bool WritePrefab(const YAML::Node& prefabYaml, const YAML::Emitter& emitter)
			{
				const UUID& prefabID = m_model.prefabID;
				std::filesystem::path prefabPath = (m_registry.currentPath / m_registry.storage[m_modelID].source.stem()).lexically_normal();
				prefabPath += ".gprefab";

				if (!WriteYamlFile(prefabPath, prefabYaml))
				{
					LOG_ERROR("failed to create prefab file ${}", m_registry.storage[m_modelID].source);
					return false;
				}

				MetaData meta;
				meta.source = prefabPath;
				meta.isStandalone = false;
				meta.generatedFrom = m_modelID;
				meta.id = prefabID;
				meta.importHash = 0;
				meta.assetType = AssetType::Prefab;
				meta.snapshotHash = SaveEmitterSnapshot(emitter, 0, prefabID.ToString());

				WritePrefabMetaFile(meta);
				m_generated.push_back({ meta.source,meta.id,AssetType::Prefab });
				m_registry.storage[prefabID] = meta;
				return true;
			}

			void PersistSourceMeta()
			{
				if (!m_model.dirtyMeta)
					return;
				m_sourceMeta["Generated Dependencies"] = m_generated;
				WriteYamlFile(m_sourceMetaPath, m_sourceMeta);
			}


			Registry& m_registry;
			const UUID m_modelID;
			ImporterCallbacks m_callbacks;

			std::filesystem::path m_sourcePath;
			std::filesystem::path m_sourceMetaPath;
			YAML::Node m_sourceMeta;

			Assimp::Importer m_importer; // owns m_scene
			const aiScene* m_scene = nullptr;

			ModelContents m_model;
			std::vector<AssetDependency> m_generated;
		};

		int HalveDimension(int value) {
			return (value >> 1) > 1 ? (value >> 1) : 1;
		}

		uint32_t ComputeMipChainSize(int width, int height, int channels, int mipCount) {
			uint32_t totalSize = width * height * channels;
			int w = width, h = height;
			for (int i = 1; i < mipCount; ++i) {
				w = HalveDimension(w);
				h = HalveDimension(h);
				totalSize += w * h * channels;
			}
			return totalSize;
		}

		bool GetPixelFormat(int channels, PixelDataFormat& format, stbir_pixel_layout& layout) {
			switch (channels)
			{
			case 1:
				format = PixelDataFormat::R8;
				layout = STBIR_1CHANNEL;
				return true;
			case 2:
				format = PixelDataFormat::RG8;
				layout = STBIR_2CHANNEL;
				return true;
			case 3:
				format = PixelDataFormat::RGB8;
				layout = STBIR_RGB;
				return true;
			case 4:
				format = PixelDataFormat::RGBA8;
				layout = STBIR_RGBA;
				return true;
			default:
				return false;
			}
		}

		// buffer already contains mip 0 and mips[0] is filled in; generates the remaining levels
		void GenerateMipChain(std::vector<unsigned char>& buffer, std::vector<MipDataPacked>& mips, int width, int height,
			int channels, int mipCount, stbir_pixel_layout layout) {
			for (int i = 1; i < mipCount; ++i) {
				int mipW = HalveDimension(width);
				int mipH = HalveDimension(height);
				mips[i].width = mipW;
				mips[i].height = mipH;
				mips[i].offset = mips[i - 1].offset + mips[i - 1].byteSize;
				mips[i].byteSize = mipW * mipH * channels;
				stbir_resize_uint8_linear(buffer.data() + mips[i - 1].offset, width, height, 0,
					buffer.data() + mips[i].offset, mipW, mipH, 0,
					layout);
				width = mipW;
				height = mipH;
			}
		}

		bool ParseShaderSource(std::istream& shadersSource, std::vector<std::pair<std::string, ShaderType>>& parsedShaders) {
			std::string currentShaderCode;
			ShaderType currentShaderType = ShaderType::None;
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
						return false;
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
			return true;
		}

	} // anonymous namespace

	std::filesystem::path AssetImporter::ImportModel(const UUID& id)
	{
		ImporterCallbacks callbacks;
		callbacks.importTexture = [this](const UUID& textureID, TextureImportSettings* settings) { return ImportTexture(textureID, settings); };
		callbacks.importMaterial = [this](const UUID& materialID) { return ImportMaterial(materialID); };
		callbacks.importPrefab = [this](const UUID& prefabID) { return ImportPrefab(prefabID); };

		ModelImportSession session(m_registry, id, std::move(callbacks));
		return session.Run();
	}

	std::filesystem::path AssetImporter::ImportTexture(const UUID& id, TextureImportSettings* settings)
	{
		TextureFileHeader header;
		std::filesystem::path filepath = m_registry.storage[id].source;
		std::string srcPath = filepath.string();
		if (!SourceFileExists(filepath))
			return std::filesystem::path();
		int width, height, nrChannels;
		unsigned char* data = stbi_load(srcPath.c_str(), &width, &height, &nrChannels, 0);
		if (!data || !settings || settings->mipcount <= 0)
		{
			LOG_ERROR("FAILED TO IMPORT : ${} ", srcPath);
			return std::filesystem::path();
		}
		const uint32_t totalSize = ComputeMipChainSize(width, height, nrChannels, settings->mipcount);
		std::vector<unsigned char> databuffer;
		databuffer.resize(totalSize);
		memcpy(databuffer.data(), data, width * height * nrChannels);
		STBI_FREE(data);

		std::vector<MipDataPacked> mips;
		mips.resize(settings->mipcount);
		header.width = width;
		header.height = height;
		mips[0] = { header.width,header.height,0, header.width * header.height * nrChannels };
		header.mipCount = settings->mipcount;
		memcpy(header.validation, "Texture", 8);
		stbir_pixel_layout layout;
		if (!GetPixelFormat(nrChannels, header.format, layout))
		{
			LOG_ERROR("INVALID NUMBER OF CHANNELS FOR ${}", srcPath);
			return std::filesystem::path();
		}
		GenerateMipChain(databuffer, mips, width, height, nrChannels, static_cast<int>(header.mipCount), layout);
		header.dataSize = totalSize;

		std::filesystem::path outputPath = CookedFile(id, "Textures", ".gtex");
		std::ofstream file;
		if (!OpenCookedFile(outputPath, file))
			return std::filesystem::path();
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		file.write(reinterpret_cast<const char*>(mips.data()), header.mipCount * sizeof(MipDataPacked));
		file.write(reinterpret_cast<const char*>(databuffer.data()), header.dataSize);
		file.close();
		return FinalizeImport(id, m_registry.storage[id], outputPath, AssetType::Texture);
	}

	std::filesystem::path AssetImporter::ImportShader(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!SourceFileExists(filepath))
			return std::filesystem::path();
		std::vector<std::pair<std::string, ShaderType>> parsedShaders;
		std::stringstream shadersSource(DumpFileToString(filepath));
		if (!ParseShaderSource(shadersSource, parsedShaders))
			return std::filesystem::path();

		std::filesystem::path outputPath = CookedFile(id, "Shaders", ".gs");
		ShaderFileHeader header;
		header.shaderCount = parsedShaders.size();
		memcpy(&header.validation, "Shader", 7);
		std::ofstream file;
		if (!OpenCookedFile(outputPath, file))
			return std::filesystem::path();
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		for (auto& [shader, type] : parsedShaders) {
			ShaderDataPacked data;
			data.size = shader.size();
			data.type = type;
			file.write(reinterpret_cast<const char*>(&data), sizeof(data));
			file.write(reinterpret_cast<const char*>(shader.data()), data.size);
		}
		file.close();
		return FinalizeImport(id, m_registry.storage[id], outputPath, AssetType::Shader);
	}

	std::filesystem::path AssetImporter::ImportMaterial(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!SourceFileExists(filepath))
			return std::filesystem::path();
		std::filesystem::path outputPath = CookedFile(id, "Materials", ".gmat");

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
		if (!node["Shader"] || !node["Tint"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return std::filesystem::path();
		}
		UUID shader = UUID(node["Shader"].as<uint64_t>(), true);
		if (!m_registry.Has(shader))
			LOG_ERROR("SHADER REFERENCED IN ${} NOT FOUND", filepath);

		CopyIntoLibrary(filepath, outputPath);
		return FinalizeImport(id, m_registry.storage[id], outputPath, AssetType::Material);
	}

	std::filesystem::path AssetImporter::ImportPrefab(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!SourceFileExists(filepath))
			return std::filesystem::path();
		std::filesystem::path outputPath = CookedFile(id, "Prefabs", ".gprefab");

		// check for missing refs (root + direct children only)
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
		WarnAboutMissingResources(root);
		for (auto child : root["Children"])
			WarnAboutMissingResources(child["Entity"]);

		CopyIntoLibrary(filepath, outputPath);
		return FinalizeImport(id, m_registry.storage[id], outputPath, AssetType::Prefab);
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