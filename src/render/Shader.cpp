#include "Render/Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
Shader::Shader(GLenum type, const std::filesystem::path& filepath) {
	std::string shaderSourceCode = DumpFileToString(filepath);
	const char* srcCodeChar = shaderSourceCode.c_str();
	m_shaderID = glCreateShader(type);
	glShaderSource(m_shaderID, 1, &srcCodeChar, nullptr);
	glCompileShader(m_shaderID);
	{ //check compilation status
		int compiled;
		glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &compiled);
		if(compiled == GL_FALSE)
		{
			int logLength = 0;
			glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &logLength);
			char log[1024];
			glGetShaderInfoLog(m_shaderID, logLength, &logLength, log);
			LOG_ERROR("Shader failed to compile \n: ", filepath, log);
		}
	}
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
	{
		int linked;
		glGetProgramiv(m_programID, GL_LINK_STATUS, &linked);

		if (linked == GL_FALSE)
		{
			int logLength = 0;
			glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLength);

			char log[1024];
			glGetProgramInfoLog(m_programID, logLength,&logLength ,log);

			LOG_ERROR("Program failed to link :\n", log);
		}
	} 
}
void ShaderProgram::SetUniformMatrix4(const std::string& name,const glm::mat4& matrix) {
	Bind();
	glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, 0,glm::value_ptr(matrix));
}
void ShaderProgram::Bind() const {
	glUseProgram(m_programID);
}