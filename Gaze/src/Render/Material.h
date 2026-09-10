#pragma once
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Resources/Asset.h"
#include "Render/Buffer.h"
namespace Gaze {
	struct MaterialBufferData {
		glm::vec4 tint;
	};
	struct MaterialData {
		UUID shader;
		UUID albedoTexture;
		glm::vec4 tint;
		MaterialData():shader(ReservedUUID::DEFAULTSHADER),albedoTexture(ReservedUUID::NONE)
		,tint(1,1,1,1){}
	};
	class Material {
	public:
		UUID shader;
		UUID albedoTexture; // temp :singular for now, later material can have multiple textures : Map["string"] - > assethandle or whatever
		glm::vec4 tint;
		Material(const MaterialData& data);
	

		static const MaterialData& GetFallbackMaterial();

		void Bind();
	private: // think about changing, idk how this should work yet, material - > ubo, or idk TODO later
		UniformBuffer m_ubo;
	};
}