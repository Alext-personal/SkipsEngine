#pragma once
#include "Render/VertexArray.h"
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Ecs/EntityRegistry.h"
class Renderer {
public:
	static void Init();
	static void OnWindowResize(uint32_t width, uint32_t height);
	static void BeginFrame();
	static void Draw(const Mesh& mesh,const Shader& shader);
	static void DrawScene(EntityRegistry& ecs); // change to scene later
	static void SetWireFrameMode(bool enabled);
private:
};