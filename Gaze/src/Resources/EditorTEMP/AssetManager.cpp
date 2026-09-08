#include "pch.h"
#include "Resources/EditorTEMP/AssetManager.h"
namespace Gaze {
	inline AssetType GetAssetTypeFromFileExtension(const std::string& extension) {
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr")
			return AssetType::Texture;
		if (extension == ".shader")
			return  AssetType::Shader;
		if (extension == ".fbx" || extension == ".obj" || extension == ".gltf" || extension == ".glb")
			return  AssetType::Source;
		if (extension == ".prefab")
			return AssetType::Prefab;
		if (extension == ".mat")
			return AssetType::Material;
		return AssetType::None;
	}
	void AssetManager::InitializeAssetsFolder() { // .meta -> memory, if !.meta, import asset, create .meta
		for (const auto& file : std::filesystem::directory_iterator(m_currentPath))
		{
			std::string extension = file.path().extension().string();
			
			MetaData meta;
			meta.assetType = GetAssetTypeFromFileExtension(extension);

			if (meta.assetType == AssetType::None) {
				LOG_WARNING("${} FileType not supported", file.path());
				continue;
			}
			std::filesystem::path metapath = file.path();
			metapath += ".meta";

			if (std::filesystem::exists(metapath))
			{
				bool error = false;
				YAML::Node metafile = YAML::LoadFile(metapath.string());
				if (meta.assetType != StringToAssetType(metafile["Type"].as<std::string>())) {
					LOG_ERROR("${} Meta file type mismatch (source and meta have different types)", metapath);
					error = true;
				}
				if(file.path().string() != metafile["Source"].as<std::string>()){
					LOG_ERROR("${} Meta file type mismatch (meta source is not  source asset path)", metapath);
					error = true;
				}
				if (!error) { // load file
					meta.source = metafile["Source"].as<std::string>();
					meta.id = metafile["UUID"].as<uint64_t>();
					meta.importHash = metafile["ImportHash"].as<uint64_t>();
					switch (meta.assetType) {
						case AssetType::Texture:
							meta.importSettings = std::make_unique<TextureImportSettings>(metafile["ImportSettings"]);
							break;
						case AssetType::Source:
							for (const auto& dependency : metafile["Generated"]) {
								uint64_t id = dependency.as<uint64_t>();
								UUID uuid = UUID(id, true);
								meta.generatedDependencies.push_back(uuid);
							}
							break;
					}
					m_assetData[meta.id] = meta;
					continue;
				}
			}
			meta.id = UUID();
			meta.source = file.path();
			meta.importSettings = nullptr;
			meta.importHash = 0; //generate it
			switch (meta.assetType) {
				case AssetType::Texture:
					meta.importSettings = std::make_unique<TextureImportSettings>();
					break;
			}

			//create meta file
		}
	}
}