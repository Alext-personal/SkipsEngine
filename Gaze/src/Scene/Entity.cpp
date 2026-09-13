#include <pch.h>
#include "Scene/Entity.h"
#include "Resources/ResourceManager.h"
namespace Gaze {
	Entity Entity::GetParent() {
		if (GetComponent<HierarchyMember>().parent == 0)
			return *this;
		return Entity(*m_scene, GetComponent<HierarchyMember>().parent);
	}
	std::vector<Entity> Entity::GetChildren() {
		std::vector<Entity> children;
		uint32_t child = GetComponent<HierarchyMember>().firstChild;
		while (child != 0)
		{
			children.emplace_back(*m_scene, child);
			child = m_scene->GetRegistry().GetComponent<HierarchyMember>(child).nextSibling;
		}
		return children;
	}
	void Entity::SetParent(Entity& parent) {
		{
			if (parent.m_entityID == m_entityID)
				return;
			HierarchyMember& hierarchy = GetComponent<HierarchyMember>();
			Transform& transform = GetComponent<Transform>();
			if (m_scene->GetTransformSystem()->IsDirty(m_entityID))
				m_scene->GetTransformSystem()->ResolveEntity(m_entityID);
			glm::mat4 currentWorldTransform = transform.GetMatrix();
			if (hierarchy.parent != 0) {
				Entity oldParent = GetParent();
				oldParent.RemoveChild(*this);
			}
			if (parent.m_entityID != 0) {
				parent.AddChild(*this);
				Transform& parentTransform = parent.GetComponent<Transform>();
				if (m_scene->GetTransformSystem()->IsDirty(parent.m_entityID))
					m_scene->GetTransformSystem()->ResolveEntity(parent.m_entityID);
				glm::mat4 newParentWorld = parentTransform.GetMatrix();
				glm::mat4 newLocal = glm::inverse(newParentWorld) * currentWorldTransform;
				transform.SetFromMatrix(newLocal);
			}
			else {
				transform.SetFromMatrix(currentWorldTransform);
			}
			m_scene->GetTransformSystem()->MarkDirty(m_entityID);
			hierarchy.parent = parent.m_entityID;
		}
	}
	void Entity::AddChild(Entity& child) {
		if (child.m_entityID == 0)
			return;
		HierarchyMember& hierarchy = GetComponent<HierarchyMember>();
		HierarchyMember& childHierarchy = child.GetComponent<HierarchyMember>();
		if (hierarchy.firstChild == 0)
		{
			hierarchy.firstChild = child.m_entityID;
			childHierarchy.parent = m_entityID;
			childHierarchy.nextSibling = 0;
			childHierarchy.prevSibling = 0;
			return;
		}
		HierarchyMember& prevSibling = m_scene->GetRegistry().GetComponent<HierarchyMember>(hierarchy.firstChild);
		childHierarchy.prevSibling = hierarchy.firstChild;
		if (prevSibling.nextSibling != 0) {
			HierarchyMember& nextSibling = m_scene->GetRegistry().GetComponent<HierarchyMember>(prevSibling.nextSibling);
			childHierarchy.nextSibling = prevSibling.nextSibling;
			nextSibling.prevSibling = child.m_entityID;
		}
		prevSibling.nextSibling = child.m_entityID;
		childHierarchy.parent = m_entityID;
	}
	void Entity::RemoveChild(Entity& child) {
		{
			if (child.m_entityID == 0)
				return;
			HierarchyMember& hierarchy = GetComponent<HierarchyMember>();
			uint32_t c = hierarchy.firstChild;
			while (c != 0)
			{
				HierarchyMember& currentHierarchy = m_scene->GetRegistry().GetComponent<HierarchyMember>(c);
				if (c == child.m_entityID)
				{
					if (c == hierarchy.firstChild)
						hierarchy.firstChild = currentHierarchy.nextSibling;
					else
						m_scene->GetRegistry().GetComponent<HierarchyMember>(currentHierarchy.prevSibling).nextSibling = currentHierarchy.nextSibling;
					if (currentHierarchy.nextSibling != 0) {
						m_scene->GetRegistry().GetComponent<HierarchyMember>(currentHierarchy.nextSibling).prevSibling = currentHierarchy.prevSibling;
					}
					currentHierarchy.parent = 0;
					currentHierarchy.prevSibling = 0;
					currentHierarchy.nextSibling = 0;
					return;
				}
				c = currentHierarchy.nextSibling;
			}
		}
	}

	void Entity::Rotate(glm::vec3 eulerAngles) {
		GetComponent<Transform>().Rotate(eulerAngles);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	void Entity::SetRotation(glm::vec3 eulerAngles) {
		GetComponent<Transform>().SetRotation(eulerAngles);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	glm::vec3 Entity::GetRotationEuler() {return GetComponent<Transform>().GetRotationEuler(); }

	void Entity::Rotate(glm::quat quat) {
		GetComponent<Transform>().Rotate(quat);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	void Entity::SetRotation(glm::quat quat) {
		GetComponent<Transform>().SetRotation(quat);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	glm::quat Entity::GetRotationQuat() { return GetComponent<Transform>().GetRotationQuat(); }

	void Entity::SetScale(glm::vec3 newscale) {
		GetComponent<Transform>().SetScale(newscale);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	void Entity::Scale(glm::vec3 newscale) {
		GetComponent<Transform>().Scale(newscale);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	glm::vec3 Entity::GetScale() { return GetComponent<Transform>().GetScale(); }

	void Entity::SetPosition(glm::vec3 pos) {
		GetComponent<Transform>().SetPosition(pos);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	void Entity::Translate(glm::vec3 pos) {
		GetComponent<Transform>().Translate(pos);
		m_scene->GetTransformSystem()->MarkDirty(m_entityID);
	}
	glm::vec3 Entity::GetPosition() { return GetComponent<Transform>().GetPosition(); }

	void Entity::SetTransform(const Transform& t) {

		SetPosition(t.GetPosition());
		SetRotation(t.GetRotationQuat());
		SetScale(t.GetScale());
	}
	void Entity::SetMesh(const UUID& meshID) {
			MeshRenderer& m = GetComponent<MeshRenderer>();
			ResourceManager::Get().RemoveRef(m.mesh);
			m.mesh = meshID;
			ResourceManager::Get().AddRef(meshID);
	}
	void Entity::SetMaterialSlot(uint32_t slot, const UUID& materialID) {
		//materials[slot ] =
			MeshRenderer& m = GetComponent<MeshRenderer>();
			ResourceManager::Get().RemoveRef(m.materials[slot]);
			m.materials[slot] = materialID;
			ResourceManager::Get().AddRef(materialID);
	}
}