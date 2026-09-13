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
		static uint64_t GetContentHash(const std::filesystem::path& path);
		static uint64_t GetContentHash(void* data,uint32_t size);
		static uint64_t GetContentHash(uint64_t firstHash, uint64_t secondHash);
	private:
		AssetRegistry& m_registry;
	};
}