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
	ResourceManager::ResourceManager() {
		ENGINE_ASSERT(s_instance != nullptr, "DUPLICATE RESOURCE MANAGER INSTANCE");
		s_instance = this;
		m_resources.resize(10); //10 types for now
		m_resources[GetResourceTypeID<Mesh>()] = std::make_unique<ResourceStorage<Mesh>>(AssetType::Mesh);
		m_resources[GetResourceTypeID<Shader>()] = std::make_unique<ResourceStorage<Shader>>(AssetType::Shader);
		m_resources[GetResourceTypeID<Texture>()] = std::make_unique<ResourceStorage<Texture>>(AssetType::Texture);
		m_resources[GetResourceTypeID<Material>()] = std::make_unique<ResourceStorage<Material>>(AssetType::Material);
		m_resources[GetResourceTypeID<Prefab>()] = std::make_unique<ResourceStorage<Prefab>>(AssetType::Prefab);
	}
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
			if (id == ReservedUUID::NONE || !HasData(id))
			{
				LOG_ERROR("Resource with UUID : ${} failed to load,  No Data  ", id);
				return std::make_shared<Mesh>(defaultMesh);
			}
			loadedMesh = ResourceLoader::LoadMesh(m_resourcesImportData[id].filepath);
		}
		return std::make_shared<Mesh>(loadedMesh);
	}
	template <>
	std::shared_ptr<Texture> ResourceManager::LoadResource(const UUID& id) {
		if (id == ReservedUUID::DEFAULTTEXTURE) {
			return std::make_shared<Texture>(Texture::GetDefaultTexture());
		}
		if (id == ReservedUUID::NONE || !HasData(id))
		{
			LOG_ERROR("Resource with UUID : ${} failed to load, No Data  ", id);
			return std::make_shared<Texture>(Texture::GetFallbackTexture());
		}
		TextureData loadedTexture = ResourceLoader::LoadTexture(m_resourcesImportData[id].filepath);
		std::shared_ptr<Texture> loadedAsset = std::make_shared<Texture>(loadedTexture);
		return loadedAsset;
	}
	template <>
	std::shared_ptr<Shader> ResourceManager::LoadResource(const UUID& id) {
		if (id == ReservedUUID::NONE || !HasData(id))
		{
			LOG_ERROR("Resource with UUID : ${} failed to load,  No Data  ", id);
			return std::make_shared<Shader>(Shader::GetFallbackShader());
		}
		std::shared_ptr<Shader> loadedAsset;
		loadedAsset = std::make_shared<Shader>(ResourceLoader::LoadShader(m_resourcesImportData[id].filepath));
		return loadedAsset;
	}
	template <>
	std::shared_ptr<Material> ResourceManager::LoadResource(const UUID& id) {
		if (id == ReservedUUID::DEFAULTMATERIAL)
			return std::make_shared<Material>(Material::GetDefaultMaterial());
		if (id == ReservedUUID::NONE || !HasData(id))
		{
			LOG_ERROR("Resource with UUID : ${} failed to load,  No Data  ", id);
			return std::make_shared<Material>(Material::GetFallbackMaterial());
		}
		MaterialData loadedMaterial = ResourceLoader::LoadMaterial(m_resourcesImportData[id].filepath);
		std::shared_ptr<Material> loadedAsset = std::make_shared<Material>(loadedMaterial);
		return loadedAsset;
	}
	template <>
	std::shared_ptr<Prefab> ResourceManager::LoadResource(const UUID& id) {
		if (id == ReservedUUID::NONE || !HasData(id))
		{
			LOG_ERROR("Resource with UUID : ${} failed to load, No Data  ", id);
			return nullptr;
		}
		Prefab loadedPrefab = ResourceLoader::LoadPrefab(m_resourcesImportData[id].filepath);
		std::shared_ptr<Prefab> loadedAsset = std::make_shared<Prefab>(loadedPrefab);

		return loadedAsset;
		// todo implement with persistent resource storage
	}
}