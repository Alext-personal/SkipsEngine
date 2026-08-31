#pragma once
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <filesystem>
namespace Gaze {
	enum class ShaderType {
		None, Vertex, Fragment
	};
	class Shader {
	public:
		Shader(const std::filesystem::path& filepath);
		~Shader();
		void SetUniformMatrix4(const std::string& name, const glm::mat4& matrix);
		void Bind() const;
		uint32_t GetID() const { return m_shaderID; }
	private:
		uint32_t m_shaderID{};
		struct ShaderCode {
			std::string src;
			ShaderType type;
		};
		static std::vector<ShaderCode> SeparateShaders(const std::filesystem::path& filepath);
		void CreateCompileAndLinkShaders(std::vector<ShaderCode>& shaders, const std::filesystem::path& filepath);
	};
}