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
		AssetRegistry m_registry;
		AssetImporter m_importer;
	};
}