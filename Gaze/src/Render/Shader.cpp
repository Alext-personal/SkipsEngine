#include "pch.h"
#include "Render/Shader.h"
#include <glad/glad.h>
namespace Gaze {
	Shader::Shader(const std::vector<ShaderData>& shaders) {
		m_shaderID = glCreateProgram();
		if (!shaders.empty())
			CreateCompileAndLinkShaders(shaders);
		else
			CreateCompileAndLinkShaders(Shader::GetFallbackShader());
		SetTextureSlots();
	}
	Shader::~Shader() {
		glDeleteProgram(m_shaderID);
	}
	void Shader::SetUniformMatrix4(const std::string& name, const glm::mat4& matrix) {
		glProgramUniformMatrix4fv(m_shaderID,glGetUniformLocation(m_shaderID, name.c_str()), 1	, 0, glm::value_ptr(matrix));
	}
	void Shader::SetTextureSlots() {
		TrySetUniformInt1("AlbedoTexture", TextureSlots::Albedo);
	}
	bool Shader::TrySetUniformInt1(const std::string& name, uint32_t value) {
		uint32_t loc = glGetUniformLocation(m_shaderID, name.c_str());
		if (loc == -1)
			return false;
		glProgramUniform1i(m_shaderID, loc, value);
		return true;
	
	}

	const std::vector<ShaderData> Shader::GetFallbackShader() {
		std::vector<ShaderData> data;
		ShaderData vertex;
		ShaderData fragment;
		vertex.type = ShaderType::Vertex;
		vertex.src = R"(#version 460 core
		layout(location = 0) in vec3 position;
		uniform mat4 modelMatrix;
		layout(std140, binding = 0) uniform Matrices {
			mat4 projection;
			mat4 view;
		};
		void main() 
		{ 
			gl_Position = projection * view * modelMatrix * vec4(position,1.0f);
		}
		)";
		fragment.type = ShaderType::Fragment;
		fragment.src = R"(#version 460 core
		out vec4 color;
		void main()
		{
			color = vec4(1.0,0.0,1.0,1.0);
		}
		)";
		data.push_back(vertex);
		data.push_back(fragment);
		return data;
	}

	void Shader::Bind() const {
		glUseProgram(m_shaderID);
	}
	void Shader::CreateCompileAndLinkShaders(const std::vector<ShaderData>& shaders) {
		std::vector<uint32_t> shadersToDelete;
		for (const ShaderData& shader : shaders) {
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
					char log[4096];
					glGetShaderInfoLog(ID, logLength, &logLength, log);
					LOG_ERROR("${} Shader failed to compile \nLog: ${} ", debugTypeName, log);
					return;
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
				return;
			}
		}
		for (uint32_t id : shadersToDelete) {
			glDeleteShader(id);
		}
	}
}