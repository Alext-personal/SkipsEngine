#pragma once
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Render/Primitives/Primitives.h"
#include <map>
class AssetManager {
public:
	static std::shared_ptr<Shader> GetShader(const std::filesystem::path& filepath);
	static std::shared_ptr<Shader> GetShader();
	static std::shared_ptr<Mesh> GetMesh(const std::filesystem::path &filepath);
	static std::shared_ptr<Mesh> GetMesh(PrimitiveType type);
private:
	static std::map<std::string,std::shared_ptr<Shader>>m_loadedShaders;	
	static std::map<std::string,std::shared_ptr<Mesh>> m_loadedMeshes;
};