#pragma once
#include <filesystem>
namespace Gaze {
	class AssetImporter {
	public:
		std::filesystem::path ImportMesh(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportTexture(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportShader(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportMaterial(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportPrefab(const std::filesystem::path& sourcePath);
	};
}