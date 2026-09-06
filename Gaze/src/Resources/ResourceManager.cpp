#include "pch.h"
#include "Resources/ResourceManager.h"
#include "Core/Log.h"
#include "Resources/Asset.h"
#include "Render/Mesh.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Resources/Prefab.h"
#include "Render/Primitives/Primitives.h"
#include "Core/Helpers.h"
#include "Resources/ResourceLoader.h"
namespace Gaze {
	template <>
	std::shared_ptr<Mesh> ResourceManager::LoadResource(const UUID& id) {
		MeshData defaultMesh = Primitives::LoadPrimitiveByType(PrimitiveType::Cube);
		MeshData loadedMesh = defaultMesh;
		if (id.GetFlag() == 0)
		{
			if (id == ReservedUUID::CUBE)
				loadedMesh = Primitives::LoadPrimitiveByType(PrimitiveType::Cube);
			if (id == ReservedUUID::QUAD)
				loadedMesh = Primitives::LoadPrimitiveByType(PrimitiveType::Quad);
			if (id == ReservedUUID::TRIANGLE)
				loadedMesh = Primitives::LoadPrimitiveByType(PrimitiveType::Triangle);
		}
		else {
			if (!HasData(id))
			{
				LOG_ERROR("${} Resource with UUID : ${} failed to load,  No Data  ", AssetTypeToString(m_resourcesImportData[id].type), id.ToString());
				return std::make_shared<Mesh>(defaultMesh);
			}
			loadedMesh = ResourceLoader::LoadMesh(m_resourcesImportData[id].filepath);
		}
		return std::make_shared<Mesh>(loadedMesh);
	}
	template <>
	std::shared_ptr<Texture> ResourceManager::LoadResource(const UUID& id) {
		TextureData loadedTexture;
		std::filesystem::path defaultTexturePath = GetCurrentPath() / "Library/Engine/Textures" / ReservedUUID::DEFAULTTEXTURE.ToString();
		defaultTexturePath += ".tex";
		if (id.GetFlag() == 0)
		{
			if (id == ReservedUUID::DEFAULTTEXTURE)
				loadedTexture = ResourceLoader::LoadTexture(defaultTexturePath);
		}
		else {
			if (!HasData(id))
			{
				LOG_ERROR("${} Resource with UUID : ${} failed to load, No Data  ", AssetTypeToString(m_resourcesImportData[id].type), id.ToString());
				return std::make_shared<Texture>(ResourceLoader::LoadTexture(defaultTexturePath));
			}
			loadedTexture = ResourceLoader::LoadTexture(m_resourcesImportData[id].filepath);
		}
		std::shared_ptr<Texture> loadedAsset = std::make_shared<Texture>(loadedTexture);
		return loadedAsset;
	}
	template <>
	std::shared_ptr<Shader> ResourceManager::LoadResource(const UUID& id) {
		std::shared_ptr<Shader> loadedAsset;
		std::filesystem::path defaultShaderPath = GetCurrentPath() / "Library/Engine/Shaders" / ReservedUUID::DEFAULTSHADER.ToString();
		defaultShaderPath += ".shader";
		if (id.GetFlag() == 0)
		{
			if (id == ReservedUUID::DEFAULTSHADER)
				loadedAsset = std::make_shared<Shader>(defaultShaderPath);
		}
		else {
			if (!HasData(id))
			{
				LOG_ERROR("${} Resource with UUID : ${} failed to load,  No Data  ", AssetTypeToString(m_resourcesImportData[id].type), id.ToString());
				return std::make_shared<Shader>(defaultShaderPath);
			}
			loadedAsset = std::make_shared<Shader>(m_resourcesImportData[id].filepath);//ResourceLoader::LoadShader(m_resourcesImportData[id].filepath); // later ,shader needs rewriting
		}
		return loadedAsset;
	} //no ResourceLoader::LoadShader yet , to implement later (custom shading language -> binary dump of all shader parts with header files ( nrshaders : 2 , vertex char count , fragment char count, vertex src, fragment src)
	template <>
	std::shared_ptr<Material> ResourceManager::LoadResource(const UUID& id) {
		std::shared_ptr<Material> loadedAsset;
		std::filesystem::path defaultMaterialPath = GetCurrentPath() / "Library/Engine/Materials" / ReservedUUID::DEFAULTMATERIAL.ToString();
		defaultMaterialPath += ".mat";
		if (id.GetFlag() == 0)
		{
			if (id == ReservedUUID::DEFAULTMATERIAL) {
				loadedAsset = std::make_shared<Material>(defaultMaterialPath);
			}
		}
		else {
			if (!HasData(id))
			{
				LOG_ERROR("${} Resource with UUID : ${} failed to load,  No Data  ", AssetTypeToString(m_resourcesImportData[id].type), id.ToString());
				return std::make_shared<Material>(defaultMaterialPath);
			}
			loadedAsset = std::make_shared<Material>(m_resourcesImportData[id].filepath);
		}
		return loadedAsset;
	}
	template <>
	std::shared_ptr<Prefab> ResourceManager::LoadResource(const UUID& id) {
		if (!HasData(id))
		{
			LOG_ERROR("${} Resource with UUID : ${} failed to load, No Data  ", AssetTypeToString(m_resourcesImportData[id].type), id.ToString());
			return nullptr;
		}
		Prefab loadedPrefab = ResourceLoader::LoadPrefab(m_resourcesImportData[id].filepath);
		std::shared_ptr<Prefab> loadedAsset = std::make_shared<Prefab>(loadedPrefab);

		return loadedAsset;
		// todo implement with persistent resource storage
	}
}