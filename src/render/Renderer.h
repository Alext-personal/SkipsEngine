#pragma once
#include "Render/VertexArray.h"
#include "Render/Shader.h"
#include "Render/Mesh.h"
class Renderer {
public:
	static void Init();
	static void OnWindowResize(uint32_t width, uint32_t height);
	static void PreDraw();
	static void Draw(const Mesh& mesh,const ShaderProgram& shader);
	static void SetWireFrameMode(bool enabled);
private:
};