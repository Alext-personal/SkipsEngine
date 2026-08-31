#pragma once
#include "Render/Material.h"
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Render/Primitives/Primitives.h"
#include "Assets/Asset.h"
#include "Core/Helpers.h"
#include "Assets/ModelLoader.h"
#include <unordered_map>
#include <yaml-cpp/yaml.h>
namespace Gaze {
	struct MetaData {
		uint32_t version;
		UUID id;
		AssetType assetType;
		std::string filepath;
	};
	template <typename T>
	struct AssetStorage { //runtime
		std::unordered_map<UUID, std::weak_ptr<T>> storage;
		AssetType type;
		AssetStorage(AssetType t) : type(t) {}
		std::weak_ptr<T> Get(const UUID& id) const {
			auto it = storage.find(id);
			if (it != storage.end())
				return it->second;
			return std::weak_ptr<T>();
		}
		void Add(const UUID& id,std::weak_ptr<T> ptr) {
			if (!ptr.expired())
				storage.emplace(id, ptr);
		}
	};
	class AssetManager {
	public:
		static void SetCurrentPath(const std::filesystem::path& filepath) { s_currentPath = filepath; }
		template <typename T>
		static std::shared_ptr<T> Get(const UUID& id){
			auto& storage = GetStorage<T>();
			std::weak_ptr<T> existingAsset = storage.Get(id);
			std::shared_ptr<T> returnedAsset;
			
			if (returnedAsset = existingAsset.lock())
				return returnedAsset;
			else {
				Asset handle(id, storage.type);
				std::shared_ptr<T> loadedAsset = Load<T>(handle);
				storage.Add(id, std::weak_ptr(loadedAsset));
				return loadedAsset;
			}
		 }
		static bool Import(std::filesystem::path filepath) {
			std::ifstream file(filepath);
			if (!file) {
				LOG_ERROR("Asset failed to be imported : ${}", filepath);
				file.close();
				return false;
			}
			std::filesystem::path metapath = filepath;
			metapath += ".meta";
			std::ifstream file2(metapath);
			if (file2)
				return false;
			file2.close();
			std::string extension = filepath.extension().string();
			MetaData meta;
			meta.assetType = AssetType::None;
			if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr")
				meta.assetType = AssetType::Texture;
			if (extension == ".shader")
				meta.assetType = AssetType::Shader;
			if (extension == ".fbx" || extension == ".obj" || extension == ".gltf" || extension == ".glb")
				meta.assetType = AssetType::Mesh;
			if (extension == ".mat")
				meta.assetType = AssetType::Material;
			if (meta.assetType == AssetType::None)
				return false;
			meta.version = 1;
			meta.id = UUID();
			meta.filepath = filepath.string();
			YAML::Node node;
			node["Asset"] = meta;
			YAML::Emitter out;
			out << node;
			std::ofstream metaFile(filepath+=".meta");
			metaFile << out.c_str();
			return true;
		}
		static void Init() { // goes through all .meta files in s_currentPath / Assets, loads into s_assetData
			for (const auto& file : std::filesystem::directory_iterator(s_currentPath))
			{
				if (file.path().extension() == ".meta")
				{
					YAML::Node node = YAML::LoadFile(file.path().string());
					MetaData metadata = node["Asset"].as<MetaData>();
					s_assetData.emplace(metadata.id, metadata);
				}
			}
		}
	private:
		template <typename T> static AssetStorage<T>& GetStorage();
		template <> static AssetStorage<Mesh>& GetStorage() { return s_meshStorage; }
		template <> static AssetStorage<Shader>& GetStorage() { return s_shaderStorage; }
		template <> static AssetStorage<Material>& GetStorage() { return s_materialStorage; }
	private:
		template <typename T> static std::shared_ptr<T> Load(const Asset& asset);
		template <typename T> static std::shared_ptr<T> LoadReserved(const Asset& asset);

		template <> static std::shared_ptr<Mesh> Load(const Asset& asset) {
			if (asset.id.GetFlag() == 0)
				return LoadReserved<Mesh>(asset);
			const auto& it = s_assetData.find(asset.id);
			if (it != s_assetData.end())
			{
				MetaData data = it->second;
				if (data.assetType != AssetType::Mesh)
				{
					LOG_ERROR("Type mismatch, Metadata is meshtype but Get<NOT MESH> was called");
					return nullptr;
				}
				return std::make_shared<Mesh>(ModelLoader::LoadModel(data.filepath));
			}
			LOG_ERROR("Mesh not found");
			return nullptr;
		}
		template <> static std::shared_ptr<Mesh> LoadReserved(const Asset& asset) {
			if (asset.id == ReservedUUID::TRIANGLE)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Triangle));
			if (asset.id == ReservedUUID::QUAD)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Quad));
			if (asset.id == ReservedUUID::CUBE)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Cube));
			LOG_ERROR("Reserved Mesh not found");
			return nullptr;
		}

		template <> static std::shared_ptr<Shader> Load(const Asset& asset) {
			if (asset.id.GetFlag() == 0)
				return LoadReserved<Shader>(asset);
			const auto& it = s_assetData.find(asset.id);
			if (it != s_assetData.end())
			{
				MetaData data = it->second;
				if (data.assetType != AssetType::Shader)
				{
					LOG_ERROR("Type mismatch, Metadata is shadertype but Get<NOT SHADER> was called");
					return nullptr;
				}
				return std::make_shared<Shader>(data.filepath);
			}
			LOG_ERROR("Shader not found");
			return nullptr;
		}
		template <> static std::shared_ptr<Shader> LoadReserved(const Asset& asset) {
			if (asset.id == ReservedUUID::DEFAULTSHADER)
				return std::make_shared<Shader>(GetCurrentPath() / "engineAssets/shaders/DefaultShader.shader");
			LOG_ERROR("Reserved Shader not found");
			return nullptr;
		}

		//add textures

		template <> static std::shared_ptr<Material> Load(const Asset& asset) {
			if (asset.id.GetFlag() == 0)
				return LoadReserved<Material>(asset);
			const auto& it = s_assetData.find(asset.id);
			if (it != s_assetData.end())
			{
				MetaData data = it->second;
				if (data.assetType != AssetType::Material)
				{
					LOG_ERROR("Type mismatch, Metadata is materialtype but Get<NOT MATERIAL> was called");
					return nullptr;
				}
				//return std::make_shared<Shader>(data.filepath); TODO IMPLEMENT MATERIALS FIRST
			}
			LOG_ERROR("Material not found");
			return nullptr;
		}
		template <> static std::shared_ptr<Material> LoadReserved(const Asset& asset) {
			LOG_ERROR("Reserved Material not found");
			return nullptr;
		}

	private:
		inline static std::filesystem::path s_currentPath{GAZE_SOURCE_ASSET_ROOT}; // hardcoded for testing purposes
		inline static std::unordered_map<UUID, MetaData> s_assetData{};
		inline static AssetStorage<Mesh> s_meshStorage{ AssetType::Mesh };
		inline static AssetStorage<Shader> s_shaderStorage{ AssetType::Shader };
		inline static AssetStorage<Material> s_materialStorage{ AssetType::Material };
	};

}
namespace YAML {
	template<>
	struct convert<Gaze::MetaData> {
		static Node encode(const Gaze::MetaData& rhs) {
			Node node;
			node["Version"] = rhs.version;
			node["Id"] = (uint64_t)rhs.id;
			node["AssetType"] = Gaze::AssetTypeToString(rhs.assetType);
			node["FilePath"] = rhs.filepath;
			return node;
		}

		static bool decode(const Node& node, Gaze::MetaData& rhs) {
			if (!node.IsMap() || node.size() != 4) {
				return false;
			}

			rhs.version = node["Version"].as<uint32_t>();
			rhs.id = Gaze::UUID(node["Id"].as<uint64_t>(), true);
			rhs.assetType = Gaze::StringToAssetType(node["AssetType"].as<std::string>());
			rhs.filepath = node["FilePath"].as<std::string>();
			return true;
		}
	};
}