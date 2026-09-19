#pragma once
#include "Core/UUID.h"
#include "Resources/Asset.h"
#include <unordered_map>
#include <queue>
#include <memory>
namespace Gaze {
	enum class TaskType {
		Load,Unload,ClearResourcesData,ClearResources
	};
	enum class ResourceImportState {
		Pending,Valid
	};
	struct ResourceImportData {
		std::filesystem::path filepath;
		AssetType type;
		ResourceImportState state = ResourceImportState::Pending;
	};
	struct Task {
		UUID id;
		std::unique_ptr<ResourceImportData> data;
		TaskType type;
	};
	struct IResourceStorage{
		AssetType type;
		IResourceStorage(AssetType _type) : type(_type){}
		virtual ~IResourceStorage() = default;
		virtual void AddRef(const UUID& id) = 0;
		virtual void RemoveRef(const UUID& id) = 0;
		virtual void Clear(const UUID& id) = 0;
		virtual bool Has(const UUID& id) = 0;
	};
	template <typename T>
	struct ref_ptr {
		std::shared_ptr<T> ptr = nullptr;
		uint32_t refcount = 0;
	};
	template <typename T>
	struct ResourceStorage : IResourceStorage  { //runtime
		std::unordered_map<UUID, ref_ptr<T>> storage;
		ResourceStorage(AssetType t) : IResourceStorage(t) {}

		std::shared_ptr<T> Get(const UUID& id) const {
			auto it = storage.find(id);
			if (it != storage.end())
				return it->second.ptr;
			return nullptr;
		}
		void Add(const UUID& id, std::shared_ptr<T> ptr) {
			storage[id].ptr = ptr;
			storage[id].refcount = 1;
		}
		void AddRef(const UUID& id) override {
			auto it = storage.find(id);
			if (it != storage.end())
				it->second.refcount++;
		}
		void RemoveRef(const UUID& id) override {
			auto it = storage.find(id);
			if (it != storage.end()) {
				if (it->second.refcount == 0)
					return;
				if (--(it->second.refcount) == 0)
					it->second.ptr = nullptr;
				
			}
		}
		void Clear(const UUID& id) override {
			auto it = storage.find(id);
			if (it != storage.end()) {
				if (it->second.refcount == 0)
					return;
				it->second.refcount = 0;
				it->second.ptr = nullptr;

			}
		}
		bool Has(const UUID& id) override {
			auto it = storage.find(id);
			if (it != storage.end() && it->second.refcount != 0)
				return true;
			return false;
		}
	};
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
			ENGINE_ASSERT(s_instance != nullptr, "NO RESOURCE MANAGER INSTANCE");
			return *s_instance;
		}
		ResourceManager();
		template <typename T>
		std::shared_ptr<T> GetResource(const UUID& id) {
			auto& storage = GetStorage<T>();

			std::shared_ptr<T> returnedAsset = storage.Get(id);

			if (returnedAsset != nullptr)
				return returnedAsset;
			else
			{
				std::shared_ptr<T> loadedAsset;
				if (m_resourcesImportData.find(id) == m_resourcesImportData.end()) {
					if (id.GetFlag() == 0) {
						if(id == ReservedUUID::DEFAULTTEXTURE)
							LOG_INFO("LOADED DEFAULT TEXTURE");
						loadedAsset = LoadResource<T>(id);
					}
					else
					{
						LOG_ERROR("RESOURCE DATA NOT FOUND WITH ID ${}", id);
						loadedAsset = LoadResource<T>(0);
					}
				}
				else {
					if (m_resourcesImportData[id].state == ResourceImportState::Pending)
						loadedAsset = LoadResource<T>(0);
					else
						loadedAsset = LoadResource<T>(id);
				}
				storage.Add(id,loadedAsset);
				return loadedAsset;
			}
		}
		bool IsResourceDataLoaded(const UUID& id) {
			auto it = m_resourcesImportData.find(id);
			if (it != m_resourcesImportData.end())
				return true;
			return false;
		}
		void ScheduleUnloadResourceData(const UUID& id) {
			auto it = m_resourcesImportData.find(id);
			if (it != m_resourcesImportData.end()) {
				m_resourcesImportData[id].state = ResourceImportState::Pending;
				m_scheduled.push({ id,nullptr,TaskType::Unload });
			}
		}
		void ScheduleLoadResourceData(const UUID& id, const ResourceImportData& data) {
			LOG_INFO("SCHEDULED RESOURCE DATA WITH UUID : ${},", id.Get());
			m_resourcesImportData[id].state = ResourceImportState::Pending;
			m_scheduled.push({ id,std::make_unique<ResourceImportData>(data),TaskType::Load });
		}
		void ScheduleUnloadResourcesData() {
			m_scheduled.push({ 0,nullptr,TaskType::ClearResourcesData });
		}
		void ScheduleUnloadResources(){
			m_scheduled.push({ 0,nullptr,TaskType::ClearResources });
		}
		void AddRef(const UUID& id) {
			for (auto& storage : m_resources)
				storage->AddRef(id);
		}
		void RemoveRef(const UUID& id) {
			for (auto& storage : m_resources)
				storage->RemoveRef(id);
		}
		void Initialize();
		void OnFrameStart();
		void RegisterPrefabCallback(std::function<void(UUID id)> callback) { m_prefabLoadedCallback = callback; }

	private:
		inline static ResourceManager* s_instance = nullptr;
		std::vector<std::unique_ptr<IResourceStorage>> m_resources;
		std::unordered_map<UUID, ResourceImportData> m_resourcesImportData;
		std::queue<Task>m_scheduled;
		std::function<void(UUID id)> m_prefabLoadedCallback;
	private:
		template <typename T>
		ResourceStorage<T>& GetStorage() {
			return *static_cast<ResourceStorage<T>*>(
				m_resources[GetResourceTypeID<T>()].get()
			);
		}
		template <typename T>
		ResourceStorage<T>& GetStorage(const UUID& id) {
			return *static_cast<ResourceStorage<T>*>(
				m_resources[GetResourceTypeID<T>()].get()
				);
		}
		bool HasData(const UUID& id){
			auto it = m_resourcesImportData.find(id);
			if (it == m_resourcesImportData.end())
			{
				LOG_ERROR("Resource with UUID : ${} failed to load, import data  not stored in memory", id);
				return false;
			}
			return true;
		}

	private:
		template <typename T>
		std::shared_ptr<T> LoadResource(const UUID& id);
	};
}