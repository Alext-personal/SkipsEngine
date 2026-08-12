#include "Assets/AssetManager.h"
#include "Assets/ModelLoader.h"
std::map<std::string, std::shared_ptr<Mesh>>AssetManager::m_loadedMeshes{};
std::map<std::string, std::shared_ptr<Shader>>AssetManager::m_loadedShaders{};
std::shared_ptr<Shader> AssetManager::GetShader(const std::filesystem::path& filepath) {
	try {
		return m_loadedShaders.at(filepath.string());
	}
	catch (std::out_of_range& e) {
		std::shared_ptr<Shader> loadedShader = std::make_shared<Shader>(filepath.string());
		m_loadedShaders[filepath.string()] = loadedShader;
		return loadedShader;
	}
}
std::shared_ptr<Shader> AssetManager::GetShader() {
	return GetShader("Gaze/assets/shaders/DefaultShader.shader");
}
std::shared_ptr<Mesh> AssetManager::GetMesh(const std::filesystem::path& filepath) {
	try {
		return m_loadedMeshes.at(filepath.string());
	}
	catch (std::out_of_range& e) {
		std::shared_ptr<Mesh> loadedMesh = std::make_shared<Mesh>(ModelLoader::LoadModel(filepath));
		m_loadedMeshes[filepath.string()] = loadedMesh;
		return loadedMesh;
	}
}
std::shared_ptr<Mesh> AssetManager::GetMesh(PrimitiveType type) {
	try {
		return m_loadedMeshes.at(PrimitiveTypeToString(type));
	}
	catch (std::out_of_range& e) {
		std::shared_ptr<Mesh> loadedMesh = std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(type));
		m_loadedMeshes[PrimitiveTypeToString(type)] = loadedMesh;
		return loadedMesh;
	}
}
