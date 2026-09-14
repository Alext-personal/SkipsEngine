#include "pch.h"
#include "Resources/EditorTEMP/AssetManager.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/ResourceManager.h"
namespace Gaze {
	inline AssetType GetAssetTypeFromFileExtension(const std::string& extension) {
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr")
			return AssetType::Texture;
		if (extension == ".gshader")
			return  AssetType::Shader;
		if (extension == ".fbx" || extension == ".obj" || extension == ".gltf" || extension == ".glb")
			return  AssetType::Source;
		if (extension == ".gprefab")
			return AssetType::Prefab;
		if (extension == ".gmat")
			return AssetType::Material;
		return AssetType::None;
	}
	void AssetManager::InitializeAssetsFolder() { // .meta -> memory, if !.meta, import asset, create .meta
		for (const auto& file : std::filesystem::directory_iterator(m_registry.currentPath))
		{
			std::string extension = file.path().extension().string();
			LOG_WARNING("AT ${}, EXTENSION IS ${}", file.path(),extension);
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
				LOG_WARNING("META FOUND, ${}" ,metapath);
				bool error = false;
				YAML::Node metafile = YAML::LoadFile(metapath.string());
				if (meta.assetType != StringToAssetType(metafile["Type"].as<std::string>())) {
					LOG_ERROR("${} Meta file type mismatch (source and meta have different types)", metapath);
					error = true;
				}
				if(file.path() != std::filesystem::path(metafile["Source"].as<std::string>())){
					LOG_ERROR("${} Meta file type mismatch (meta source is not  source asset path) : ${}   ${}", metapath,file.path().generic_string(),metafile["Source"].as<std::string>());
					error = true;
				}
				if (!error) { // load file
					meta.importSettings = nullptr;
					meta.source = metafile["Source"].as<std::string>();
					meta.id = metafile["UUID"].as<uint64_t>();
					meta.importHash = metafile["ImportHash"].as<uint64_t>();
					meta.snapshotHash = metafile["SnapshotHash"].as<uint64_t>();
					if(metafile["Generated Dependencies"])
						meta.generatedDependencies = metafile["Generated Dependencies"].as<std::vector<AssetDependency>>(); //implement YAML conversion
					meta.isStandalone = metafile["Standalone"].as<bool>();
					meta.generatedFrom = metafile["Generated From"].as<uint64_t>();
					switch (meta.assetType) {
						case AssetType::Texture:
							meta.importSettings = std::make_unique<TextureImportSettings>(metafile["ImportSettings"]);
							break;
					}
					LOG_WARNING("META FILE LOADED IN MEMORY WITH ID ${} : ",meta.id.Get());
					m_registry.storage[meta.id] = meta;
					if(meta.assetType == AssetType::Source)
						m_priorityImports[meta.id] = meta;
					continue;
				}
			}
			meta.isStandalone = true;
			meta.generatedFrom = 0;
			meta.id = UUID();
			meta.source = file.path().lexically_normal();
			meta.importSettings = nullptr;
			meta.importHash = 0; //generate it on actual import
			meta.snapshotHash = 0;
			YAML::Node metafile;
			metafile["Source"] = meta.source.string();
			metafile["UUID"] = meta.id.Get();
			metafile["Type"] = AssetTypeToString(meta.assetType);
			metafile["Generated From"] = 0;
			metafile["SnapshotHash"] = 0;
			metafile["ImportHash"] = meta.importHash;
			metafile["Standalone"] = true;
			switch (meta.assetType) {
				case AssetType::Texture:
					meta.importSettings = std::make_unique<TextureImportSettings>();
					metafile["ImportSettings"] = meta.importSettings->Serialize();
					break;
			}
			m_registry.storage[meta.id] = meta;
			if (meta.assetType == AssetType::Source)
				m_priorityImports[meta.id] = meta;
			std::ofstream fl(metapath);
			fl << metafile;
			fl.close();
			continue;
		}
	}
	void AssetManager::LoadAssets() {
		for (auto& [id,asset]:m_priorityImports) {
			m_importer.ImportModel(id);
		}
		for (auto& [id,asset] : m_registry.storage) {
			switch (asset.assetType)
			{
			case AssetType::Texture:
				if (!ResourceManager::Get().IsResourceDataLoaded(id))
					m_importer.ImportTexture(id, static_cast<TextureImportSettings*>(asset.importSettings.get()));
				break;
			case AssetType::Shader:
				if (!ResourceManager::Get().IsResourceDataLoaded(id))
					m_importer.ImportShader(id);
				break;
			case AssetType::Material:
				if (!ResourceManager::Get().IsResourceDataLoaded(id))
					m_importer.ImportMaterial(id);
				break;
			case AssetType::Source:
				continue;
			case AssetType::Prefab:
				if(!ResourceManager::Get().IsResourceDataLoaded(id))
					m_importer.ImportPrefab(id);
				break;
			default:
				break;
			}
		}
	}
}