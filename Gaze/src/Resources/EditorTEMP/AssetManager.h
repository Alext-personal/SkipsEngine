#pragma once
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/EditorTEMP/AssetWatcher.h"
#include <filesystem>
namespace Gaze {
	class AssetManager {
	public:
		AssetManager() :m_importer(m_registry),m_watcher(m_registry) {}
		void InitializeAssetsFolder();
		UUID ProcessFile(const std::filesystem::path& filepath,const UUID& id = 0);
		void ProcessMovedFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);
		void ProcessMovedDirectory(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);
		void ProcessRemovedFile(const std::filesystem::path& path);
		void LoadAssets();
		void ImportAsset(const UUID& id);
		void ProcessQueue();
	private:
		std::unordered_map<UUID, MetaData> m_priorityImports;
		AssetRegistry m_registry;
		AssetImporter m_importer;
		AssetWatcher m_watcher;
	};
}