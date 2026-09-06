#pragma once
#include "Core/UUID.h"
#include "Resources/Asset.h"
#include <unordered_map>
#include <memory>
namespace Gaze {
	struct ResourceImportData {
		std::filesystem::path filepath;
		AssetType type;
	};
	struct IResourceStorage{
		AssetType type;
		IResourceStorage(AssetType _type) : type(_type){}
		virtual ~IResourceStorage() = default;
	};
	template <typename T>
	struct ResourceStorage : IResourceStorage  { //runtime
		std::unordered_map<UUID, std::weak_ptr<T>> storage;
		ResourceStorage(AssetType t) : IResourceStorage(t) {}

		std::weak_ptr<T> Get(const UUID& id) const {
			auto it = storage.find(id);
			if (it != storage.end())
				return it->second;
			return std::weak_ptr<T>();
		}
		void Add(const UUID& id, std::weak_ptr<T> ptr) {
			if (!ptr.expired())
				storage[id] = ptr;
		}
	};
	template <typename T>
	struct PersistentResourceStorage : IResourceStorage {

	}; // todo implement (for prefabs for example , prefab = lightweight handle over scene hierarchy
	inline uint8_t GetLastResourceTypeID() {
		static uint8_t globalResourceTypeID = 0;
		return globalResourceTypeID++;
	}
	template <typename T>
	inline uint8_t GetResourceTypeID() {
		static uint8_t specificResourceTypeID = GetLastResourceTypeID();
		return specificResourceTypeID;
	}
	class ResourceManager {
	public:
		static ResourceManager& Get() {
			ENGINE_ASSERT(!s_instance, "NO RESOURCE MANAGER INSTANCE");
			return *s_instance;
		}
		ResourceManager() {
			m_resources.resize(10); //10 types for now
			m_resources[GetResourceTypeID<Mesh>()] = std::make_unique<ResourceStorage<Mesh>>(AssetType::Mesh);
			m_resources[GetResourceTypeID<Shader>()] = std::make_unique<ResourceStorage<Shader>>(AssetType::Shader);
			m_resources[GetResourceTypeID<Texture>()] = std::make_unique<ResourceStorage<Texture>>(AssetType::Texture);
			m_resources[GetResourceTypeID<Material>()] = std::make_unique<ResourceStorage<Material>>(AssetType::Material);
			m_resources[GetResourceTypeID<Prefab>()] = std::make_unique<ResourceStorage<Prefab>>(AssetType::Prefab);
		}
		template <typename T>
		std::shared_ptr<T> GetResource(const UUID& id) {
			auto& storage = GetStorage<T>();

			std::weak_ptr<T> existingAsset = storage.Get(id);
			std::shared_ptr<T> returnedAsset;

			if (returnedAsset = existingAsset.lock())
				return returnedAsset;
			else
			{
				std::shared_ptr<T> loadedAsset = LoadResource<T>(id);
				if(loadedAsset!= nullptr)
					storage.Add(id, std::weak_ptr(loadedAsset));
				return loadedAsset;
			}
		}
		void SetResourceImportData(const UUID& id, const ResourceImportData& data) { m_resourcesImportData[id] = data; }

	private:
		inline static ResourceManager* s_instance = nullptr;
		std::vector<std::unique_ptr<IResourceStorage>> m_resources;
		std::unordered_map<UUID, ResourceImportData> m_resourcesImportData;
	private:
		template <typename T>
		ResourceStorage<T>& GetStorage() {
			return *static_cast<ResourceStorage<T>*>(
				m_resources[GetResourceTypeID<T>()].get()
			);
		}
		template <typename T>
		PersistentResourceStorage<T>& GetPersistentStorage() {
			return *static_cast<PersistentResourceStorage<T>*>(
				m_resources[GetResourceTypeID<T>()].get()
				);
		}
		bool HasData(const UUID& id){
			auto it = m_resourcesImportData.find(id);
			if (it == m_resourcesImportData.end())
			{
				LOG_ERROR("Resource with UUID : ${} failed to load, import data  not stored in memory", id.ToString());
				return false;
			}
			return true;
		}

	private:
		template <typename T>
		std::shared_ptr<T> LoadResource(const UUID& id);
	};
}