#include "pch.h"
#include "Resources/EditorTEMP/AssetManager.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/ResourceManager.h"
namespace Gaze {
	namespace {
		AssetType GetAssetTypeFromFileExtension(const std::string& extension) {
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
		bool LoadMetaFile(const std::filesystem::path& filepath,MetaData& out) {
			if (std::filesystem::exists(filepath))
			{
				YAML::Node metafile;
				try {
					metafile = YAML::LoadFile(filepath.string());
				}
				catch (const YAML::Exception& e) {
					LOG_ERROR("YAML FILE FAILED TO OPEN :${},  ${}", filepath, e.what());
					return false;
				}
				out.assetType = StringToAssetType(metafile["Type"].as<std::string>());
				out.importSettings = nullptr;
				out.source = metafile["Source"].as<std::string>();
				out.id = metafile["UUID"].as<uint64_t>();
				out.importHash = metafile["ImportHash"].as<uint64_t>();
				out.snapshotHash = metafile["SnapshotHash"].as<uint64_t>();
				if (metafile["Generated Dependencies"])
					out.generatedDependencies = metafile["Generated Dependencies"].as<std::vector<AssetDependency>>(); 
				out.isStandalone = metafile["Standalone"].as<bool>();
				out.generatedFrom = metafile["Generated From"].as<uint64_t>();
				switch (out.assetType) {
				case AssetType::Texture:
					out.importSettings = std::make_unique<TextureImportSettings>(metafile["ImportSettings"]);
					break;
				}
				return true;
			}
			return false;
		}
		void CreateMetaData(const std::filesystem::path& srcPath, MetaData& out) {

			out.isStandalone = true;
			out.generatedFrom = 0;
			out.id = UUID();
			out.source = srcPath.lexically_normal();
			out.importSettings = nullptr;
			out.importHash = 0; //generate it on actual import
			out.snapshotHash = 0;
			switch (out.assetType) {
			case AssetType::Texture:
				out.importSettings = std::make_unique<TextureImportSettings>();
				break;
			}
		}
		bool WriteMetaYAML(const MetaData& in,YAML::Node& out) {
			out["Source"] = in.source.string();
			out["UUID"] = in.id.Get();
			out["Type"] = AssetTypeToString(in.assetType);
			out["Generated From"] = 0;
			out["SnapshotHash"] = 0;
			out["ImportHash"] = in.importHash;
			out["Standalone"] = true;
			switch (in.assetType) {
			case AssetType::Texture:
				out["ImportSettings"] = in.importSettings->Serialize();
				break;
			}
		}
		
	}
	void AssetManager::ProcessFile(const std::filesystem::path& filepath) {
		std::string extension = filepath.extension().string();
		AssetType typeFromExtension = GetAssetTypeFromFileExtension(extension);
		if (typeFromExtension == AssetType::None)
			return;

		std::filesystem::path metapath = filepath;
		metapath += ".meta";

		MetaData meta;
		bool success = LoadMetaFile(metapath, meta);
		if (success) {
			if (typeFromExtension != meta.assetType)
			{
				LOG_WARNING("META FILE ASSET TYPE DIFFERS FROM FILE ASSET TYPE ${}   ${}", metapath, filepath);
				return;
			}
		}
		else
		{
			meta.assetType = typeFromExtension;
			CreateMetaData(filepath, meta);

			YAML::Node metafile;
			WriteMetaYAML(meta, metafile);
			std::ofstream fl(metapath);
			fl << metafile;
			fl.close();
		}
		m_registry.storage[meta.id] = meta;
		if (meta.assetType == AssetType::Source)
			m_priorityImports[meta.id] = meta;
	}
	void AssetManager::InitializeAssetsFolder() { // .meta -> memory, if !.meta, import asset, create .meta
		for (const auto& file : std::filesystem::recursive_directory_iterator(m_registry.currentPath))
		{
			if (file.is_directory())
				continue;
			ProcessFile(file.path());
		}
	}
	void AssetManager::LoadAssets() {
		for (auto& [id,asset]:m_priorityImports) {
			if(m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
				m_importer.ImportModel(id);

		}
		for (auto& [id,asset] : m_registry.storage) {
			if (asset.isStandalone == false && asset.assetType != AssetType::Prefab)
				continue;
			switch (asset.assetType)
			{
			case AssetType::Texture:
				if (!ResourceManager::Get().IsResourceDataLoaded(id) || m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
					m_importer.ImportTexture(id, static_cast<TextureImportSettings*>(asset.importSettings.get()));
				break;
			case AssetType::Shader:
				if (!ResourceManager::Get().IsResourceDataLoaded(id) || m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
					m_importer.ImportShader(id);
				break;
			case AssetType::Material:
				if (!ResourceManager::Get().IsResourceDataLoaded(id) || m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
					m_importer.ImportMaterial(id);
				break;
			case AssetType::Source:
				continue;
			case AssetType::Prefab:
				if (!ResourceManager::Get().IsResourceDataLoaded(id) || m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
				{
					m_importer.ImportPrefab(id);
				}
				break;
			default:
				break;
			}
		}
		for (auto& [id, asset] : m_priorityImports) {
			for (auto& dep : asset.generatedDependencies) {
				if (!m_registry.Has(dep.id) && dep.type == AssetType::Prefab)
					m_importer.ImportModel(id);
			}
		}
	}

}