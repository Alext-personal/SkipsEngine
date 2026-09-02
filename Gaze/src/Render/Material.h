#pragma once
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Assets/Asset.h"
#include "Render/Buffer.h"
namespace Gaze {
	struct MaterialBufferData {
		glm::vec4 tint;
	};
	class Material {
	public:
		AssetHandle<Shader> shader;
		AssetHandle<Texture> albedoTexture; // temp :singular for now, later material can have multiple textures : Map["string"] - > assethandle or whatever
		glm::vec4 tint;
		Material();
		Material(const std::filesystem::path& filepath);
		void LoadDefaults();
		bool LoadFromFile(const std::filesystem::path& filepath);
		void SaveToFile(const std::filesystem::path& filepath);
		void Bind();
	private: // think about changing, idk how this should work yet, material - > ubo, or idk TODO later
		UniformBuffer m_ubo;
	};
}