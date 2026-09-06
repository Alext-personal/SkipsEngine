#pragma once
#include "Core/Helpers.h"
#include "Resources/Asset.h"
#include <unordered_map>
#include <yaml.h>
namespace Gaze {
	struct IImportSettings{
		virtual ~IImportSettings();
		virtual void Serialize(YAML::Emitter& out) const = 0;
		virtual void DeSerialize(const YAML::Node& in) = 0
	};
	struct MetaData {
		UUID id;
		AssetType type;
		std::vector<UUID> generatedDependencies;
		std::vector<UUID> standaloneDependencies;
		std::unique_ptr<IImportSettings> importSettings;
	};
	class AssetManager {
	public:

		static void Init() { // goes through all .meta files in s_currentPath / Assets, loads into s_assetData
			for (const auto& file : std::filesystem::directory_iterator(s_currentPath))
			{
				if (file.path().extension() == ".meta")
				{
					YAML::Node node = YAML::LoadFile(file.path().string());
					MetaData metadata = node["Asset"].as<MetaData>();
					s_assetData.emplace(metadata.id, metadata);
				}
				else
				{
					MetaData meta;
					meta.version = 1;
					meta.id = UUID();
					meta.filepath = file.path().string();
					meta.assetType = AssetType::None;
					std::string extension = file.path().extension().string();
					if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr")
						meta.assetType = AssetType::Texture;
					if (extension == ".shader")
						meta.assetType = AssetType::Shader;
					if (extension == ".fbx" || extension == ".obj" || extension == ".gltf" || extension == ".glb")
						meta.assetType = AssetType::Model;
					if (extension == ".mat")
						meta.assetType = AssetType::Material;
					if (meta.assetType == AssetType::None)
						continue;
					std::filesystem::path metapath = file.path();
					metapath += ".meta";
					if (std::filesystem::exists(metapath))
						continue; //meta file already exists
					//write meta file
					YAML::Node node;
					node["Asset"] = meta;
					YAML::Emitter out;
					out << node;
					std::ofstream metaFile(metapath);
					metaFile << out.c_str();
				}
			}
		}
	private:
		inline static std::filesystem::path s_currentPath{GAZE_SOURCE_ASSET_ROOT}; // hardcoded for testing purposes
		inline static std::unordered_map<UUID, MetaData> s_assetData{};
	};

}