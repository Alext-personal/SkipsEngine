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
		m_resources.resize(5);
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
		if (id == ReservedUUID::NONE) {
			return std::make_shared<Mesh>(defaultMesh);
		}
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
			if (!HasData(id)) {
				LOG_ERROR("Resource with UUID : ${} failed to load,  No Data  ", id);
				return std::make_shared<Mesh>(defaultMesh);
			}
			loadedMesh = ResourceLoader::LoadMesh(m_resourcesImportData[id].filepath);
		}
		return std::make_shared<Mesh>(loadedMesh);
	}
	template <>
	std::shared_ptr<Texture> ResourceManager::LoadResource(const UUID& id) {
		if (id == ReservedUUID::NONE)
			return std::make_shared<Texture>(Texture::GetFallbackTexture());
		if (id == ReservedUUID::DEFAULTTEXTURE) {
			return std::make_shared<Texture>(Texture::GetDefaultTexture());
		}
		if (!HasData(id))
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
		if (id == ReservedUUID::NONE)
			return std::make_shared<Shader>(Shader::GetFallbackShader());
		if (!HasData(id))
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
		if(id == ReservedUUID::NONE)
			return std::make_shared<Material>(Material::GetFallbackMaterial());
		if (!HasData(id))
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
		if(id == ReservedUUID::NONE)
			return std::make_shared<Prefab>(Prefab::GetFallbackPrefab());
		if (!HasData(id))
		{
			LOG_ERROR("Resource with UUID : ${} failed to load, No Data  ", id);
			return std::make_shared<Prefab>(Prefab::GetFallbackPrefab());
		}
		LOG_ERROR("GOT HERE PREFAB LOADER");
		Prefab loadedPrefab = ResourceLoader::LoadPrefab(m_resourcesImportData[id].filepath);
		std::shared_ptr<Prefab> loadedAsset = std::make_shared<Prefab>(loadedPrefab);

		return loadedAsset;
		// todo implement with persistent resource storage
	}
	void ResourceManager::Initialize() {
		std::filesystem::path cookedPaths[2] = { GetCurrentPath() / "Library" / "Client" , GetCurrentPath() / "Library" / "Engine" };
		for (const auto& path : cookedPaths) {
			if (!std::filesystem::exists(path))
				continue;
			for (const auto& file : std::filesystem::recursive_directory_iterator(path)) {
				if (file.is_directory())
					continue;
				std::string extension = file.path().extension().string();
				UUID id = std::stoull(file.path().stem().string());
				AssetType type = AssetType::None;
				if (extension == ".gmat") 
					type = AssetType::Material;
				if (extension == ".gmesh")
					type = AssetType::Mesh;
				if (extension == ".gs")
					type = AssetType::Shader;
				if (extension == ".gprefab")
					type = AssetType::Prefab;
				if (extension == ".gtex")
					type = AssetType::Texture;
				if (type == AssetType::None) {
					LOG_ERROR("INVALID COOKED FILE EXTENSION, SKIPPING ${}", extension);
					continue;
				}
				m_resourcesImportData[id] = { file.path(),type,ResourceImportState::Valid };
			}
		}
	}
	void ResourceManager::OnFrameStart() {
		while(!m_scheduled.empty()) {
			auto& task = m_scheduled.front();
			switch (task.type)
			{
			case TaskType::Load:
				m_resourcesImportData[task.id] = *task.data;
				m_resourcesImportData[task.id].state = ResourceImportState::Valid;
				for (auto& storage : m_resources) {
					if (storage->Has(task.id)) {
						storage->Clear(task.id);
						if (storage->type == AssetType::Prefab)
							m_prefabLoadedCallback(task.id);
					}
				}
				break;
			case TaskType::Unload:
				if (auto it = m_resourcesImportData.find(task.id) != m_resourcesImportData.end())
					m_resourcesImportData.erase(it);
				for (auto& storage : m_resources)
					storage->Clear(task.id);
				break;
			case TaskType::ClearResourcesData:
				m_resourcesImportData.clear();
				m_resources.clear();
				break;
			case TaskType::ClearResources:
				m_resources.clear();
				break;
			}
			m_scheduled.pop();
		}
	}
}