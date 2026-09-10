#pragma once
#include <filesystem>
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
namespace Gaze {
	class AssetManager {
	public:
		AssetManager() :m_importer(m_registry) {}
		void InitializeAssetsFolder();
		void LoadAssets();
	private:
		std::filesystem::path m_currentPath{ GAZE_SOURCE_ASSET_ROOT }; // hardcoded for testing purposes
		AssetRegistry m_registry;
		AssetImporter m_importer;
	};
}