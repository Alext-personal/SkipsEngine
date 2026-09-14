
#include "pch.h"
#include "Render/Material.h"
#include "Resources/ResourceManager.h"
#include <yaml-cpp/yaml.h>
namespace Gaze {
	Material::Material(const MaterialData& data) :m_ubo(sizeof(MaterialBufferData), 1) {
		shader = data.shader;
		albedoTexture = data.albedoTexture;
		tint = data.tint;
		MaterialBufferData ndata = { tint };
		m_ubo.SetData(&ndata, sizeof(MaterialBufferData), 0);
	}

	const MaterialData& Material::GetFallbackMaterial() {
		static const MaterialData fallback = [] {
			MaterialData data;
			data.albedoTexture = ReservedUUID::NONE;
			data.shader = ReservedUUID::NONE;
			data.tint = { 1,1,1,1 };
			return data;
			}();
		return fallback;
	}
	const MaterialData& Material::GetDefaultMaterial() {
		static const MaterialData defaultMat = [] {
			MaterialData data;
			data.albedoTexture = ReservedUUID::DEFAULTTEXTURE;
			data.shader = ReservedUUID::DEFAULTSHADER;
			data.tint = { 1,1,1,1 };
			return data;
			}();
		return defaultMat;
	}

	void Material::Bind() {
		m_ubo.Bind(1);
		ResourceManager::Get().GetResource<Texture>(albedoTexture)->Bind(TextureSlots::Albedo);
	}
}
