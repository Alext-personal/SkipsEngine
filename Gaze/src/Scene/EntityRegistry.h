#pragma once
#include "Scene/Component.h"
#include <vector>
namespace Gaze {
	#define NO_COMPONENT UINT32_MAX
	struct IRegistry {
		virtual ~IRegistry() = default;
		virtual void RemoveComponent(const uint32_t entity) = 0;
	};
	template <typename T>
	class ComponentRegistry : public IRegistry {
	public:
		ComponentType type;
		std::vector<T> components;
		std::vector<uint32_t> sparse; // sparse[entity] = where in components we can find entity's component 
		std::vector<uint32_t> denseEntities; // denseEntities[componentIndex] = entity that owns it
		ComponentRegistry(ComponentType _type) : type(_type) {
			components.reserve(5000);
			sparse.reserve(5000);
			denseEntities.reserve(5000);
		}
		void RemoveComponent(const uint32_t entity) override {
			if (sparse[entity] == NO_COMPONENT || entity >= sparse.size())
				return;
			const uint32_t index = sparse[entity];
			denseEntities[index] = denseEntities.back();
			components[index] = std::move(components.back());

			const uint32_t movedComponent = denseEntities[index];
			sparse[movedComponent] = index;

			denseEntities.pop_back();
			components.pop_back();

			sparse[entity] = NO_COMPONENT;
		}
	};
	class EntityRegistry {
	public:
		EntityRegistry() = default;
		uint32_t CreateEntity(const UUID& persistentID = ReservedUUID::NONE) {
			uint32_t eID;
			if (m_deletedEntities.empty()) {
				eID = m_lastEntityID++;
				m_entities.push_back(eID);
			}
			else
			{
				eID = m_deletedEntities.back();
				m_deletedEntities.pop_back();
				m_entities.push_back(eID);
			}
			if (eID >= m_entitiesPersistent.size())
				m_entitiesPersistent.resize(eID + 1);
			if (persistentID == ReservedUUID::NONE) {
				UUID persistent;
				m_entitiesPersistent[eID] = persistent;
				m_entitiesPersistentLookup[persistent] = eID;
			}
			else {
				m_entitiesPersistent[eID] = persistentID;
				m_entitiesPersistentLookup[persistentID] = eID;
			}
			LOG_INFO("Created Entity with UUID : ${}", GetUUID(eID));
			return eID;
		}
		auto DeleteEntity(uint32_t entity) {
			LOG_INFO("Deleted Entity with UUID: ${}", GetUUID(entity));
			std::apply([&](auto&&... args) {
				((args.RemoveComponent(entity)), ...);
				}, m_storages);
			UUID id = m_entitiesPersistent[entity];
			m_entitiesPersistentLookup[id] = 0;
			m_entitiesPersistent[entity] = ReservedUUID::NONE;
			m_deletedEntities.push_back(entity);
			auto it = std::find(m_entities.begin(), m_entities.end(), entity);
			return m_entities.erase(it);
		}
		~EntityRegistry() {
			for (auto it = m_entities.begin(); it != m_entities.end();)
			{
				it = DeleteEntity(*it);
			}
		}
		UUID& GetUUID(uint32_t entity) {
			return m_entitiesPersistent[entity];
		}
		template <typename T>
		T& AddComponent(const uint32_t entity) {
			if (HasComponent<T>(entity))
				return GetComponent<T>(entity);
			auto& storage = GetComponentStorage<T>();
			LOG_INFO("Added ${} component to Entity with UUID : ${} ", ComponentTypeToString(storage.type), GetUUID(entity));
			const uint32_t index = storage.components.size();
			storage.components.emplace_back();
			if (entity >= storage.sparse.size())
				storage.sparse.resize(entity + 1, NO_COMPONENT);

			storage.sparse[entity] = index;
			storage.denseEntities.push_back(entity);

			return storage.components[index];
		}
		template <typename T>
		T& GetComponent(const uint32_t entity) {
			if (!HasComponent<T>(entity))
				CLIENT_ASSERT(0, "NO COMPONENT FOUND IN ENTITY ${}", entity);
			auto& storage = GetComponentStorage<T>();
			//LOG_INFO("Fetched ${} component from Entity with UUID : ${} ", ComponentTypeToString(storage.type), GetUUID(entity));
			const uint32_t index = storage.sparse[entity];
			return storage.components[index];
		}
		template <typename T, typename Q>
		std::vector<std::pair<T*, Q*>> Get() {
			auto& TStorage = GetComponentStorage<T>();
			auto& QStorage = GetComponentStorage<Q>();
			uint32_t TSize = TStorage.components.size();
			uint32_t QSize = QStorage.components.size();
			std::vector<std::pair<T*, Q*>> returnedComponents{};
			if (TSize <= QSize) // search the smallest container, and pick the ones that also have T
				for (uint32_t index = 0; index < TSize; ++index) {
					uint32_t entity = TStorage.denseEntities[index];
					if (QStorage.sparse[entity] != NO_COMPONENT)
						returnedComponents.emplace_back(&TStorage.components[index], &QStorage.components[QStorage.sparse[entity]]);
				}
			else
				for (uint32_t index = 0; index < QSize; ++index) {
					uint32_t entity = QStorage.denseEntities[index];
					if (TStorage.sparse[entity] != NO_COMPONENT)
						returnedComponents.emplace_back(&TStorage.components[TStorage.sparse[entity]], &QStorage.components[index]);
				}
			return returnedComponents;
		}
		template <typename T>
		void RemoveComponent(const uint32_t entity) {
			auto& storage = GetComponentStorage<T>();
			storage.RemoveComponent(entity);
			LOG_INFO("Removed ${} component from Entity with UUID : ${} ", ComponentTypeToString(storage.type), GetUUID(entity));
		}
		template <typename T>
		bool HasComponent(const uint32_t entity) {
			auto& storage = GetComponentStorage<T>();
			if (entity >= storage.sparse.size()) 
				return false;
			const uint32_t index = storage.sparse[entity];
			if (index >= storage.denseEntities.size()) 
				return false;
			return storage.denseEntities[index] == entity;
		}
	private:
		uint32_t m_lastEntityID{1};
		std::vector<uint32_t> m_deletedEntities{};
		std::vector<uint32_t> m_entities{};
		std::vector<UUID> m_entitiesPersistent{};// persistent[entityID] = its UUID;
		std::unordered_map<UUID,uint32_t> m_entitiesPersistentLookup{};
		std::tuple<
			ComponentRegistry<Transform>,
			ComponentRegistry<MeshRenderer>,
			ComponentRegistry<HierarchyMember>
				  > m_storages{
			ComponentRegistry<Transform>{ComponentType::Transform},
			ComponentRegistry<MeshRenderer>{ComponentType::MeshRenderer},
			ComponentRegistry<HierarchyMember>{ComponentType::HierarchyMember}
		};
	private:
		template <typename T>
		auto& GetComponentStorage() {
			return std::get<ComponentRegistry<T>>(m_storages);
		}
	};
}