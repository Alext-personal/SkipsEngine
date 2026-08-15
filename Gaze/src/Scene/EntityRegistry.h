	#pragma once
	#include <vector>
	#include "Scene/Component.h"
	#define NO_COMPONENT UINT32_MAX
	struct IRegistry {
		virtual ~IRegistry() = default;
		virtual void RemoveComponent(const uint32_t entity) = 0;
	};
	template <typename T>
	class ComponentRegistry : public IRegistry {
	public:
		std::vector<T> components;
		std::vector<uint32_t> sparse; // sparse[entity] = where in components we can find entity's component 
		std::vector<uint32_t> denseEntities; // denseEntities[componentIndex] = entity that owns it
		void RemoveComponent(const uint32_t entity) override {
			const uint32_t index = sparse[entity];
			denseEntities[index] = denseEntities.back();
			components[index] = std::move(components.back());

			const uint32_t movedComponent = denseEntities[index];
			sparse[movedComponent] = sparse[index];

			denseEntities.pop_back();
			components.pop_back();

			sparse[index] = NO_COMPONENT;
		}
	};
	class EntityRegistry {
	public:
		uint32_t CreateEntity() {
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
			AddComponent<Transform>(eID); // entity has mandatory transform migrate to entity class for ease of use
			return eID;
		}
		void DeleteEntity(uint32_t entity){
			std::apply([&](auto&&... args) {
				((args.RemoveComponent(entity)),...);
				}, m_storages);
			auto it = std::find(m_entities.begin(), m_entities.end(), entity);
			*it = 0;
			m_deletedEntities.push_back(entity);
		}
		template <typename T>
		T& AddComponent(const uint32_t entity) {
			if(HasComponent<T>(entity))
				return GetComponent<T>(entity);
			auto& storage = GetComponentStorage<T>();
			const uint32_t index = storage.components.size();
			storage.components.emplace_back(); 
			if (entity >= storage.sparse.size())
				storage.sparse.resize(entity + 1,NO_COMPONENT);

			storage.sparse[entity] = index;
			storage.denseEntities.push_back(entity);

			return storage.components[index];
		}
		template <typename T>
		T& GetComponent(const uint32_t entity) {
			if (!HasComponent<T>(entity))
				CLIENT_ASSERT(0,"NO COMPONENT FOUND");
			auto& storage = GetComponentStorage<T>();
			const uint32_t index = storage.sparse[entity];
			return storage.components[index];
		}
		template <typename T,typename Q>
		std::vector<std::pair<T&, Q&>> Get()  {
			auto& TStorage = GetComponentStorage<T>();
			auto& QStorage = GetComponentStorage<Q>();
			uint32_t TSize = TStorage.components.size();
			uint32_t QSize = QStorage.components.size();
			std::vector<std::pair<T&, Q&>> returnedComponents{};
			if(TSize <= QSize) // search the smallest container, and pick the ones that also have T
				for (uint32_t index = 0; index < TSize; ++index) {
					uint32_t entity = TStorage.denseEntities[index];
					if (QStorage.sparse[entity] != NO_COMPONENT)
						returnedComponents.emplace_back(TStorage.components[index], QStorage.components[QStorage.sparse[entity]]);
				}
			else
				for (uint32_t index = 0; index < QSize; ++index) {
					uint32_t entity = QStorage.denseEntities[index];
					if (TStorage.sparse[entity] != NO_COMPONENT)
						returnedComponents.emplace_back(TStorage.components[TStorage.sparse[entity]], QStorage.components[index]);
				}
			return returnedComponents;
		}
		template <typename T>
		void RemoveComponent(const uint32_t entity) {
			auto& storage = GetComponentStorage<T>();
			storage.RemoveComponent(entity);
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
		uint32_t m_lastEntityID{};
		std::vector<uint32_t> m_deletedEntities{};
		std::vector<uint32_t> m_entities{};
		std::tuple <ComponentRegistry<Transform>,
					ComponentRegistry<MeshRenderer>
		>m_storages{};
	private:
		template <typename T>
		auto& GetComponentStorage() {
			return std::get<ComponentRegistry<T>>(m_storages);
		}
	};