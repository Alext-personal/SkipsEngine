#pragma once
#include <filesystem>
namespace Gaze {
	struct AssetRegistry;
	struct TextureImportSettings;
	class AssetImporter {
	public:
		AssetImporter(AssetRegistry& registry) : m_registry(registry) {}
		std::filesystem::path ImportModel(const UUID& id);
		std::filesystem::path ImportTexture(const UUID& id,TextureImportSettings* settings);
		std::filesystem::path ImportShader(const UUID& id);
		std::filesystem::path ImportMaterial(const UUID& id);
		std::filesystem::path ImportPrefab(const UUID& id);
		uint64_t GetImportHash() const;
	private:
		AssetRegistry& m_registry;
	};
}