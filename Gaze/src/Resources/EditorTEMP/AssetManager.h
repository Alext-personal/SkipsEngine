#pragma once
#include "Resources/Asset.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include <unordered_map>
namespace Gaze {
	struct MetaData {
		std::filesystem::path source;
		UUID id;
		uint64_t importHash;
		AssetType assetType;
		std::vector<UUID> generatedDependencies;
		std::unique_ptr<IImportSettings> importSettings;

		MetaData(const MetaData& other) {
			source = other.source;
			id = other.id;
			importHash = other.importHash;
			assetType = other.assetType;
			generatedDependencies = other.generatedDependencies;
			importSettings = std::make_unique<IImportSettings>(*other.importSettings);

		}
		MetaData() = default;
		MetaData(MetaData&&) = default;
		MetaData& operator=(MetaData&&) = default;
		MetaData& operator=(const MetaData& other) {
		}
	};
	class AssetManager {
	public:
		void InitializeAssetsFolder();
	private:
		std::filesystem::path m_currentPath{ GAZE_SOURCE_ASSET_ROOT }; // hardcoded for testing purposes
		std::unordered_map<UUID, MetaData> m_assetData{};
	}
};