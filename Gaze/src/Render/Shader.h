#pragma once
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <filesystem>
namespace Gaze {
	namespace TextureSlots {
		inline const uint32_t Albedo = 0;
	}
	enum class ShaderType : uint8_t {
		None, Vertex, Fragment
	};
	struct ShaderData {
		std::string src;
		ShaderType type;
	};
	class Shader {
	public:
		Shader(const std::vector<ShaderData>& shaders);
		~Shader();
		void SetUniformMatrix4(const std::string& name, const glm::mat4& matrix);
		void SetTextureSlots();
		bool TrySetUniformInt1(const std::string& name, uint32_t value);

		static const std::vector<ShaderData> GetFallbackShader();

		void Bind() const;
		uint32_t GetID() const { return m_shaderID; }
	private:
		uint32_t m_shaderID{};
		void CreateCompileAndLinkShaders(const std::vector<ShaderData>& shaders);
	};
}