#include "Render/Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace Gaze {
	Shader::Shader(const std::filesystem::path& filepath) {
		m_shaderID = glCreateProgram();
		std::vector<ShaderCode> shaders = SeparateShaders(filepath);
		CreateCompileAndLinkShaders(shaders, filepath);
	}
	Shader::~Shader() {
		glDeleteProgram(m_shaderID);
	}
	void Shader::SetUniformMatrix4(const std::string& name, const glm::mat4& matrix) {
		Bind();
		glUniformMatrix4fv(glGetUniformLocation(m_shaderID, name.c_str()), 1, 0, glm::value_ptr(matrix));
	}
	void Shader::Bind() const {
		glUseProgram(m_shaderID);
	}
	std::vector<Shader::ShaderCode> Shader::SeparateShaders(const std::filesystem::path& filepath) {
		std::vector<ShaderCode> returnedShaders{};
		std::string shaderCode;
		ShaderType currentType = ShaderType::None;
		std::stringstream shadersSource(DumpFileToString(filepath));
		std::string line;
		while (std::getline(shadersSource, line)) {
			std::stringstream identifierLine(line);
			std::string identifier;
			identifierLine >> identifier;
			if (identifier == "#type")
			{
				std::string type;
				identifierLine >> type;
				if (!(type == "Vertex" || type == "Fragment")) {
					LOG_ERROR("INVALID SHADER TYPE, CHECK SHADER SOURCE CODE");
					std::vector<ShaderCode> noShaders;
					return noShaders;
				}
				if (currentType != ShaderType::None) {
					returnedShaders.push_back({ shaderCode,currentType });
					currentType = ShaderType::None;
					shaderCode.clear();
				}
				if (type == "Vertex")
					currentType = ShaderType::Vertex;
				if (type == "Fragment")
					currentType = ShaderType::Fragment;
			}
			else {
				shaderCode += line + "\n";
			}
		}
		if (currentType != ShaderType::None) {
			returnedShaders.push_back({ shaderCode,currentType });
		}
		return returnedShaders;
	}
	void Shader::CreateCompileAndLinkShaders(std::vector<ShaderCode>& shaders, const std::filesystem::path& filepath) {
		std::vector<uint32_t> shadersToDelete;
		for (const ShaderCode& shader : shaders) {
			GLenum glType;
			std::string debugTypeName;
			if (shader.type == ShaderType::Vertex) {
				glType = GL_VERTEX_SHADER;
				debugTypeName = "Vertex";
			}
			if (shader.type == ShaderType::Fragment) {
				glType = GL_FRAGMENT_SHADER;
				debugTypeName = "Fragment";
			}

			uint32_t ID = glCreateShader(glType);
			const char* srcCode = shader.src.c_str();
			glShaderSource(ID, 1, &srcCode, nullptr);
			glCompileShader(ID);
			{ //check compilation status
				int compiled;
				glGetShaderiv(ID, GL_COMPILE_STATUS, &compiled);
				if (compiled == GL_FALSE)
				{
					int logLength = 0;
					glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &logLength);
					char log[1024];
					glGetShaderInfoLog(ID, logLength, &logLength, log);
					LOG_ERROR("${} Shader failed to compile \nFilepath: ${} \nLog: ${} ", debugTypeName, filepath, log);
				}
			}
			shadersToDelete.push_back(ID);
			glAttachShader(m_shaderID, ID);
		}
		glLinkProgram(m_shaderID);
		{
			int linked;
			glGetProgramiv(m_shaderID, GL_LINK_STATUS, &linked);

			if (linked == GL_FALSE)
			{
				int logLength = 0;
				glGetProgramiv(m_shaderID, GL_INFO_LOG_LENGTH, &logLength);

				char log[1024];
				glGetProgramInfoLog(m_shaderID, logLength, &logLength, log);

				LOG_ERROR("Program failed to link :\nLog: ${}", log);
			}
		}
		for (uint32_t id : shadersToDelete) {
			glDeleteShader(id);
		}
	}
}