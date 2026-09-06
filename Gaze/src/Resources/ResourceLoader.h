#pragma once
#include <filesystem>
namespace Gaze {
	struct Meshdata;
	struct TextureData;
	struct Prefab;
	class ResourceLoader {
	public:
		static MeshData LoadMesh(const std::filesystem::path& filepath);
		static TextureData LoadTexture(const std::filesystem::path& filepath);
		static Prefab LoadPrefab(const std::filesystem::path filepath);
	};
}