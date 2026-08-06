#pragma once
#include <string>
#include <glad/glad.h>
#include "core/Log.h"
class Shader {
public:
	Shader();
	~Shader();
private:
	uint32_t m_shaderID{};
};
class ShaderProgram {
private:
	uint32_t m_programID{};

};