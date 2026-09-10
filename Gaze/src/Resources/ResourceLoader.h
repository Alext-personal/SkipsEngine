#pragma once
#include <filesystem>
namespace Gaze {
	struct MeshData;
	struct TextureData;
	struct MaterialData;
	struct ShaderData;
	struct Prefab;
	class ResourceLoader {
	public:
		static MeshData LoadMesh(const std::filesystem::path& filepath);
		static TextureData LoadTexture(const std::filesystem::path& filepath);
		static Prefab LoadPrefab(const std::filesystem::path& filepath);
		static MaterialData LoadMaterial(const std::filesystem::path& filepath);
		static std::vector<ShaderData> LoadShader(const std::filesystem::path& filepath);
	};
}