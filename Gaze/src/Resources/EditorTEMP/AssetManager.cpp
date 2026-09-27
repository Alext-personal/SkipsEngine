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
		void CreateMetaData(const std::filesystem::path& srcPath, MetaData& out,const UUID& id) {

			out.isStandalone = true;
			out.generatedFrom = 0;
			if (id == 0)
				out.id = UUID();
			else
				out.id = id;
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
		void WriteMetaYAML(const MetaData& in,YAML::Node& out) {
			out["Source"] = in.source.string();
			out["UUID"] = in.id.Get();
			out["Type"] = AssetTypeToString(in.assetType);
			out["Generated From"] = in.generatedFrom.Get();
			out["SnapshotHash"] = in.snapshotHash;
			out["ImportHash"] = in.importHash;
			out["Standalone"] = in.isStandalone;
			switch (in.assetType) {
			case AssetType::Texture:
				out["ImportSettings"] = in.importSettings->Serialize();
				break;
			}
		}
		std::filesystem::path GetCookedPath(const UUID& id, const AssetType type) {
			std::string assetFolder;
			std::string extension;
			switch (type) {
				case AssetType::Mesh:
					assetFolder = "Meshes";
					extension = ".gmesh";
					break;
				case AssetType::Material:
					assetFolder = "Materials";
					extension = ".gmat";	
					break;
				case AssetType::Shader:
					assetFolder = "Shaders";
					extension = ".gs";
					break;
				case AssetType::Texture:
					assetFolder = "Textures";
					extension = ".gtex";
					break;
				case AssetType::Prefab:
					assetFolder = "Prefabs";
					extension = ".gprefab";
					break;
				default:
					LOG_ERROR("[GetCookedPath] SHOULDN'T BE HERE !");
					return "";
			}
			std::filesystem::path cookedPath = GetCurrentPath() / "Client" / assetFolder / id.ToString();
			cookedPath += extension;
			return cookedPath;
		}
		
	}
	UUID AssetManager::ProcessFile(const std::filesystem::path& filepath,const UUID& id) {
		std::string extension = filepath.extension().string();
		AssetType typeFromExtension = GetAssetTypeFromFileExtension(extension);
		if (typeFromExtension == AssetType::None)
			return 0;

		std::filesystem::path metapath = filepath;
		metapath += ".meta";

		MetaData meta;
		bool success = LoadMetaFile(metapath, meta);
		if (success) {
			if (typeFromExtension != meta.assetType)
			{
				LOG_WARNING("META FILE ASSET TYPE DIFFERS FROM FILE ASSET TYPE ${}   ${}", metapath, filepath);
				return 0;
			}
			meta.source = filepath;
		}
		else
		{
			meta.assetType = typeFromExtension;
			CreateMetaData(filepath, meta,id);
		}
		YAML::Node metafile;
		WriteMetaYAML(meta, metafile);
		std::ofstream fl(metapath);
		fl << metafile;
		fl.close();
		m_registry.Add(meta.id, meta);
		if (meta.assetType == AssetType::Source)
			m_priorityImports[meta.id] = meta;
		return meta.id;
	}
	void AssetManager::ImportAsset(const UUID& id) {
		if (id == 0)
			return;
		if (!m_registry.Has(id))
		{
			LOG_WARNING("ID IS NOT REGISTERED (META FILE MISSING / CORRUPTED) ${}", id);
			return;
		}
		auto& asset = m_registry.storage[id];
		if (asset.isStandalone == false && asset.assetType != AssetType::Prefab)
			return;
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
			if (m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
				m_importer.ImportModel(id);
			return;
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
	void AssetManager::InitializeAssetsFolder() { // .meta -> memory, if !.meta, import asset, create .meta
		for (const auto& file : std::filesystem::recursive_directory_iterator(m_registry.currentPath))
		{
			if (file.is_directory())
				continue;
			ProcessFile(file.path());
		}
	}
	void AssetManager::ProcessRemovedFile(const std::filesystem::path& path) {
		if (std::filesystem::exists(path)) {
			ImportAsset(ProcessFile(path));
			return;
		}
		if (path.extension().string() == ".meta")
		{
			std::filesystem::path file = path.parent_path() / path.stem();
			if (std::filesystem::exists(file))
			{
				UUID id = m_registry.Get(file);
				ProcessFile(file, id);
				return;
			}
		}
		UUID id = m_registry.Get(path);
		if (id == 0)
			return;
		auto it = m_registry.storage.find(id);
		if (it == m_registry.storage.end())
			return;
		AssetType type = it->second.assetType;
		m_registry.storage.erase(it);
		auto it2 = m_priorityImports.find(id);
		if (it2 != m_priorityImports.end())
			m_priorityImports.erase(it2);
		ResourceManager::Get().ScheduleUnloadResourceData(id);
		std::filesystem::path cookedPath = GetCookedPath(id, type);
		if (std::filesystem::exists(cookedPath))
			std::filesystem::remove(cookedPath);
		std::filesystem::path metaPath = path;
		metaPath += ".meta";
		if (std::filesystem::exists(metaPath))
			std::filesystem::remove(metaPath);
	}
	void AssetManager::ProcessMovedFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
		if (std::filesystem::is_directory(oldPath)) {
			ProcessMovedDirectory(oldPath, newPath);
			return;
		}
		if (oldPath.extension() == ".meta")
		{
			LOG_WARNING("MODIFIED META FILE , REGENERATING, THIS META IS NOW STALE : ${}", newPath);
			ProcessFile(newPath.parent_path() / newPath.stem());
			return;
		}
		AssetType type = GetAssetTypeFromFileExtension(oldPath.extension().string());
		if (type == AssetType::None)
			return;
		std::filesystem::path oldMetaPath = oldPath;
		oldMetaPath += ".meta";
		UUID id = m_registry.Get(oldPath);
		std::filesystem::path newMetaPath = newPath;
		newMetaPath += ".meta";
		std::filesystem::rename(oldMetaPath, newMetaPath);
		ProcessFile(newPath, id);
	}
	void AssetManager::ProcessMovedDirectory(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
		for (const auto& file : std::filesystem::recursive_directory_iterator(newPath))
		{
			if (file.is_directory())
				continue;
			
			std::filesystem::path fileOldPath = oldPath / std::filesystem::relative(file.path(), newPath);
			ProcessMovedFile(fileOldPath,file.path());
		}
	}
	void AssetManager::ProcessQueue() {
		std::queue<AssetFileData> copy;
		{
			const std::lock_guard<std::mutex> lock(m_registry.importQueueMutex);
			copy = m_registry.importQueue;
			while (!m_registry.importQueue.empty())
				m_registry.importQueue.pop();
		}
		if (copy.empty())
			return;
		while (!copy.empty()) {
			auto& processed = copy.front();
			switch (processed.state) {
				case AssetFileState::Created:
				case AssetFileState::Modified:
					LOG_INFO("PROCESSING CREATED/MODIFIED");
					ImportAsset(ProcessFile(processed.path));
					break;
				case AssetFileState::Deleted:
					LOG_INFO("PROCESSING DELETED");
					ProcessRemovedFile(processed.path);
					break;
				case AssetFileState::Moved:
					LOG_INFO("PROCESSING MOVED");
					ProcessMovedFile(processed.oldPath, processed.path);
					break;
			}
			copy.pop();
		}
	}
	void AssetManager::LoadAssets() {
		std::vector<UUID> toImport;
		for (auto& [id,asset]:m_priorityImports) {
			if (m_registry.storage[id].importHash != AssetImporter::GetContentHash(m_registry.storage[id].source))
				toImport.push_back(id);
		}
		for (auto& [id,asset] : m_registry.storage) {
			if (asset.assetType == AssetType::Source)
				continue;
			toImport.push_back(id);
		}
		for (auto& id : toImport)
			ImportAsset(id);
		toImport.clear();
		for (auto& [id, asset] : m_priorityImports) {
			for (auto& dep : asset.generatedDependencies) {
				if (!m_registry.Has(dep.id) && dep.type == AssetType::Prefab) {
					toImport.push_back(id);
					break;
				}
			}
		}
		for (auto& id : toImport)
			ImportAsset(id);
		toImport.clear();
		m_watcher.Watch();
	}

}