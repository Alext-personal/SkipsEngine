#include "render/Shader.h"
Shader::Shader(GLenum type, const std::string& filepath) {
	std::string shaderSourceCode = filepath; //DumpToString(filepath) helper function
	const char* srcCodeChar = shaderSourceCode.c_str();
	m_shaderID = glCreateShader(type);
	glShaderSource(m_shaderID, 1, &srcCodeChar, nullptr);
	glCompileShader(m_shaderID);
}
Shader::~Shader() {
	glDeleteShader(m_shaderID);
}
void ShaderProgram::AttachShader(const Shader& shader) {
	glAttachShader(m_programID, shader.GetID());
}
ShaderProgram::ShaderProgram(const Shader& vertex, const Shader& fragment) {
	m_programID = glCreateProgram();
	AttachShader(vertex);
	AttachShader(fragment);
	glLinkProgram(m_programID);
}
void ShaderProgram::Bind() const {
	glUseProgram(m_programID);
}