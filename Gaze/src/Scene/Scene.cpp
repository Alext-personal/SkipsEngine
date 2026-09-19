#include "pch.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Render/Renderer.h"
#include "Render/EditorCamera.h"
#include "Resources/ResourceManager.h"
#include "Render/Material.h"
#include "Resources/Prefab.h"
namespace Gaze {
	Scene::Scene() : m_editor(true) {
		m_systems.push_back(std::make_unique<TransformSystem>(&m_entities));
	}
	void Scene::Initialize() {
		ResourceManager::Get().RegisterPrefabCallback([this](UUID id) {ResolveInstances(id); });
	}
	void Scene::OnUpdate(float dt) {
		for (auto& system : m_systems) {
			system->OnUpdate(dt);
		}
		OnRender();
	}
	void Scene::OnRender() {
		CameraUniformPass pass;
		if (m_editor)
			pass = { EditorCamera::GetProjectionMatrix(),EditorCamera::GetViewMatrix() };
		//todo else
		Renderer::SetUniformBuffer(pass); // once per frame
		for (auto& [transform, meshRenderer] : m_entities.Get<Transform, MeshRenderer>()) {
			Renderer::Draw(*transform, *meshRenderer);
		}
	}
	void Scene::Unload() {
		ResourceManager::Get().ScheduleUnloadResources();
	}
	inline Entity SpawnPrefabNode(Scene& scene, const UUID& prefabID, const UUID instanceID, const UUID& nodeID) {
		const Prefab& prefab = *ResourceManager::Get().GetResource<Prefab>(prefabID);
		Entity ent(scene);
		Instantiated& inst = ent.AddComponent<Instantiated>();
		inst.instanceID = instanceID;
		inst.nodeID = nodeID;
		inst.prefabID = prefabID;
		const EntityData& pdata = prefab.data.at(nodeID);
		ent.SetTransform(pdata.transform);
		if (pdata.meshID != 0)
		{
			ent.AddComponent<MeshRenderer>();
			ent.SetMesh(pdata.meshID);
			for (uint32_t matIndex = 0; matIndex < pdata.materialIDs.size(); ++matIndex) {
				ent.SetMaterialSlot(matIndex, pdata.materialIDs.at(matIndex));

			}
		}
		return ent;
	}
	inline void ApplyPrefabDiff(const Prefab& prefab, Entity& e, const UUID& nodeID) {

		const EntityData& pdata = prefab.data.at(nodeID);
		e.SetTransform(pdata.transform);
		if (pdata.meshID != 0)
		{
			if (!e.HasComponent<MeshRenderer>())
				e.AddComponent<MeshRenderer>();
			e.SetMesh(pdata.meshID);
			for (uint32_t matIndex = 0; matIndex < pdata.materialIDs.size(); ++matIndex)
				e.SetMaterialSlot(matIndex, pdata.materialIDs.at(matIndex));
		}
		else {
			if (e.HasComponent<MeshRenderer>())
				e.RemoveComponent<MeshRenderer>();
		}
	}
	Entity Scene::Instantiate(const UUID& prefabID) {
		UUID instanceID;
		std::shared_ptr<Prefab> prefab = ResourceManager::Get().GetResource<Prefab>(prefabID);

		std::unordered_map<UUID,Entity> entitiesCreated;
		UUID rootID = 0;
		for (auto& [nodeID, prefabNode] : prefab->data) {
			entitiesCreated[nodeID] = SpawnPrefabNode(*this, prefabID, instanceID, nodeID);
		}
		for (auto& [nodeID, prefabNode] : prefab->data) {
			if (prefabNode.parentID != 0)
				entitiesCreated[nodeID].SetParent(entitiesCreated[prefabNode.parentID]);
			else {
				rootID = nodeID;
			}
		}
		return entitiesCreated[rootID];
	}
	void Scene::ResolveInstances(const UUID& prefabID)
	{
		LOG_INFO("RESOLVING PREFAB INSTANCES . RESOLVING FOR PREFAB WITH ID ${}", prefabID);
		std::shared_ptr<Prefab> prefab = ResourceManager::Get().GetResource<Prefab>(prefabID);
		if (!prefab)
			return;

		std::unordered_map<UUID, std::unordered_set<UUID>> presentNodesByInstance;

		std::vector<uint32_t> toDestroy;
		using NodeKey = std::pair<UUID, UUID>; // instanceID, nodeID
		struct NodeKeyHash {
			size_t operator()(const NodeKey& k) const {
				return std::hash<UUID>()(k.first) ^ (std::hash<UUID>()(k.second) << 1);
			}
		};
		std::unordered_map<NodeKey, Entity,NodeKeyHash> entitiesModified;
		for (uint32_t ent : m_entities.View<Instantiated>())
		{

			Entity e(*this, ent);
			Instantiated& inst = e.GetComponent<Instantiated>();

			if (inst.prefabID != prefabID)
				continue;
			presentNodesByInstance.emplace(inst.instanceID, std::unordered_set<UUID>{});

			if (auto it = prefab->data.find(inst.nodeID); it != prefab->data.end()) {
				ApplyPrefabDiff(*prefab, e, it->first);
				presentNodesByInstance[inst.instanceID].insert(inst.nodeID);
				entitiesModified[{inst.instanceID,it->first}] = e;
			}
			else
				toDestroy.push_back(ent);
		}
		for (auto& [instanceID, presentNodes] : presentNodesByInstance)
		{
			for (auto& [key,value] : prefab->data)
				if (presentNodes.empty() || presentNodes.find(key) == presentNodes.end())
					entitiesModified[{instanceID,key}] = SpawnPrefabNode(*this, prefabID, instanceID, key);
		}
		for (auto& [instanceID, presendNodes] : presentNodesByInstance) {
			for (auto& [nodeID, prefabNode] : prefab->data) {
				if (prefabNode.parentID == 0) 
					continue;
				Entity& child = entitiesModified.at({ instanceID, nodeID });
				Entity& parent = entitiesModified.at({ instanceID, prefabNode.parentID });
				uint32_t oldParent = child.GetComponent<HierarchyMember>().parent;
				if (oldParent != parent.GetNativeID())
					child.SetParent(parent);
			}
			
		}
		for (uint32_t ent : toDestroy)
		{
			Entity e(*this, ent);
			e.Destroy();
		}
	}
}