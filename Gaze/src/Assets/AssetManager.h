#pragma once
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Render/Primitives/Primitives.h"
#include "Core/UUID.h"
#include <map>
/*enum class AssetType {
	Mesh,Material,Shader
};
struct IAssetStorage {
	AssetType type;
	IAssetStorage(AssetType t):type(t){}
};
template <typename T>
struct AssetStorage : IAssetStorage{
	std::unordered_map<UUID, std::weak_ptr<T>> storage;
	std::weak_ptr<T> Get(const UUID& id) const  {
		auto it = storage.find(id);
		if (it != storage.end())
			return it->second;

	}
};*/
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