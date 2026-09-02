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
			for (auto [transform, hierarchy] : m_registry->Get<Transform, HierarchyMember>()) {
				if (transform->isDirty) {
					auto [tr, hr] = GetHighestDirty(transform, hierarchy);
					ResolveTransforms(tr, hr);
				}
			}
		}
		void ResolveEntity(uint32_t entity) override {
			Transform* transform = &m_registry->GetComponent<Transform>(entity);
			HierarchyMember* hierarchy = &m_registry->GetComponent<HierarchyMember>(entity);
			auto [tr, hr] = GetHighestDirty(transform, hierarchy);
			ResolveTransforms(tr, hr);
		}
	private:
		std::pair<Transform*,HierarchyMember*> GetHighestDirty(Transform* transform,HierarchyMember* hierarchy) {
			Transform* lastTransform = transform;
			HierarchyMember* lastHierarchy = hierarchy;
			HierarchyMember* traversalHierarchy = hierarchy;
			while (traversalHierarchy->parent != 0)
			{
				uint32_t parent = traversalHierarchy->parent;
				Transform& parentTransform = m_registry->GetComponent<Transform>(parent);
				HierarchyMember& parentHierarchy = m_registry->GetComponent<HierarchyMember>(parent);
				if (parentTransform.isDirty) {
					lastTransform = &parentTransform;
					lastHierarchy = &parentHierarchy;
				}
				traversalHierarchy = &parentHierarchy;
			}
			return { lastTransform,lastHierarchy };
		}
		void ResolveTransforms(Transform* transform, HierarchyMember* hierarchy) {
			if (hierarchy->parent == 0) {
				transform->ComputeMatrix();
			}
			else
			{
				Transform& parentTransform = m_registry->GetComponent<Transform>(hierarchy->parent);
				transform->ComputeMatrix(parentTransform.GetMatrix());
			}
			transform->isDirty = false;
			for (uint32_t child : hierarchy->children) 
				ResolveTransforms(&m_registry->GetComponent<Transform>(child), &m_registry->GetComponent<HierarchyMember>(child));
		}
	};
}