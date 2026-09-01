
#include "pch.h"
#include "Render/Material.h"
#include "Assets/AssetManager.h"
#include <yaml-cpp/yaml.h>
namespace YAML {
	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(YAML::EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();

			return true;
		}
	};
}
namespace Gaze {
	Material::Material() {
		LoadDefaults();
	}
	Material::Material(const std::filesystem::path& filepath) {
		if (!LoadFromFile(filepath))
			LoadDefaults();
	}

	void Material::LoadDefaults()
	{
		shader.id = ReservedUUID::DEFAULTSHADER;
		shader.asset = AssetManager::Get<Shader>(shader.id);
		albedoTexture.id = ReservedUUID::DEFAULTTEXTURE;
		albedoTexture.asset = AssetManager::Get<Texture>(albedoTexture.id);
		tint = glm::vec4(1.0f);
	}

	bool Material::LoadFromFile(const std::filesystem::path& filepath)
	{
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return false;
		}
		if (!file["Shader"] || !file["Texture"] || !file["Tint"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return false;
		}
		shader.id = UUID(file["Shader"].as<uint64_t>(), true);
		shader.asset = AssetManager::Get<Shader>(shader.id);
		albedoTexture.id = UUID(file["Texture"].as<uint64_t>(), true);
		albedoTexture.asset = AssetManager::Get<Texture>(albedoTexture.id);
		tint = file["Tint"].as<glm::vec4>();
		return true;
	}
	void Material::SaveToFile(const std::filesystem::path& filepath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Shader" << YAML::Value << (uint64_t)shader.id;
		out << YAML::Key << "Texture" << YAML::Value << (uint64_t)albedoTexture.id;
		out << YAML::Key << "Tint" << YAML::Value << YAML::Node(tint);
		out << YAML::EndMap;
		std::ofstream file(filepath.string());
		file << out.c_str();
		file.close();
	}
	void Material::Bind() {
		shader.asset->Bind(); // bind material scalars ubo (tint ...);
		albedoTexture.asset->Bind(TextureSlots::Albedo);
	}
}
