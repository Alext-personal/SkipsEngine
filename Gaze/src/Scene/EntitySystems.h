#pragma once
#include "Scene/EntityRegistry.h"
namespace Gaze {
	class EntitySystem {
	public:
		EntitySystem(EntityRegistry* registry) : m_registry(registry){}
		virtual void OnUpdate(float dt) = 0;
		virtual void ResolveEntity(uint32_t entity) = 0; 
	protected:
		EntityRegistry* m_registry = nullptr;
	};
	class TransformSystem : public EntitySystem {
	public:
		TransformSystem(EntityRegistry* registry) :EntitySystem(registry){}
		void OnUpdate(float dt) override {
			ResolveDirty();
		}
		void ResolveEntity(uint32_t entity) override {
			ResolveTransforms(GetHighestDirty(entity));
		}
		void MarkDirty(uint32_t entity) {
			if (entity >= m_dirty.size())
				m_dirty.resize(entity + 1,false);
			if (m_dirty[entity] == true)
				return;
			m_dirty[entity] = true;
			m_dirtyEntities.emplace_back(entity);
		}
		void UnMarkDirty(uint32_t entity) {
			if (entity >= m_dirty.size())
				return;
			if (m_dirty[entity] == false)
				return;
			m_dirty[entity] = false;
		}
		bool IsDirty(uint32_t entity) const {
			if (entity >= m_dirty.size())
				return false;
			return m_dirty[entity];
		}
	private:
		uint32_t GetHighestDirty(uint32_t startEntity) {
			uint32_t processedEntity = startEntity;
			uint32_t highestDirtyEntity = startEntity;
			HierarchyMember* processedEntityHierarchy = &m_registry->GetComponent<HierarchyMember>(processedEntity);
			while (processedEntityHierarchy->parent != 0)
			{
				uint32_t parent = processedEntityHierarchy->parent;
				
				if (IsDirty(parent)) {
					highestDirtyEntity = parent;
				}
				processedEntity = parent;
				processedEntityHierarchy = &m_registry->GetComponent<HierarchyMember>(processedEntity);
			}
			return  highestDirtyEntity;
		}
		void ResolveTransforms(uint32_t entity) {
			Transform& currentTransform = m_registry->GetComponent<Transform>(entity);
			HierarchyMember& currentHierarchy = m_registry->GetComponent<HierarchyMember>(entity);
			if (currentHierarchy.parent == 0) {
				currentTransform.ComputeMatrix();
			}
			else
			{
				Transform& parentTransform = m_registry->GetComponent<Transform>(currentHierarchy.parent);
				currentTransform.ComputeMatrix(parentTransform.GetMatrix());
			}
			UnMarkDirty(entity);
			for (uint32_t child : currentHierarchy.children)
				ResolveTransforms(child);
		}
		void ResolveDirty() {
			for (uint32_t entity : m_dirtyEntities) {
				if (m_dirty[entity])
					ResolveEntity(entity);
			}
			m_dirtyEntities.clear();
		}
	private:
		std::vector<uint32_t> m_dirtyEntities;
		std::vector<bool> m_dirty; // index = entity
	};
}