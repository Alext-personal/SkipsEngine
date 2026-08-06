#pragma once
#include "render/VertexArray.h"
#include "render/Shader.h"
class Renderer {
public:
	static void Init();
	static void OnWindowResize(uint32_t width, uint32_t height);
	static void PreDraw();
	static void Draw(const VertexArray& vao,const ShaderProgram& shader);
private:
};