#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include "core/Log.h"
class Shader {
public:
	Shader(GLenum type, const std::string& filepath);
	~Shader();
	uint32_t GetID() const { return m_shaderID; }
private:
	uint32_t m_shaderID{};
};
class ShaderProgram {
public:
	ShaderProgram(const Shader& vertex,const Shader& fragment);
	void AttachShader(const Shader& shader);
	void Bind() const;
	uint32_t GetID() const { return m_programID; }
private:
	uint32_t m_programID{};
};