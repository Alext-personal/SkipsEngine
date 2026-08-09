#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include "Core/Log.h"
#include "Core/Helpers.h"
class Shader {
public:
	Shader(GLenum type, const std::filesystem::path& filepath);
	~Shader();
	uint32_t GetID() const { return m_shaderID; }
private:
	uint32_t m_shaderID{};
};
class ShaderProgram {
public:
	ShaderProgram(const Shader& vertex,const Shader& fragment);
	void AttachShader(const Shader& shader);
	void SetUniformMatrix4(const std::string& name,const glm::mat4& matrix);
	void Bind() const;
	uint32_t GetID() const { return m_programID; }
private:
	uint32_t m_programID{};
};