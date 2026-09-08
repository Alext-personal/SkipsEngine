#pragma once
#include <filesystem>
namespace Gaze {
	class YAML::Emitter;
	class YAML::Node;
	struct IImportSettings {
		virtual ~IImportSettings();
		virtual void Serialize(YAML::Emitter& out) const = 0;
		virtual void DeSerialize(const YAML::Node& in) = 0;
	};
	class AssetImporter {
	public:
		std::filesystem::path ImportMesh(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportTexture(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportShader(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportMaterial(const std::filesystem::path& sourcePath);
		std::filesystem::path ImportPrefab(const std::filesystem::path& sourcePath);
	};
}