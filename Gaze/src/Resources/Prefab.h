#pragma once
#include "Core/UUID.h"
#include "Scene/Component.h"
#include <unordered_map>
#include <vector>
namespace Gaze {
	struct EntityData {
		UUID meshID = ReservedUUID::NONE;
		Transform transform = {};
		std::vector<UUID>materialIDs = {};
		UUID parentID = ReservedUUID::NONE;
	};
	struct Prefab {
		std::unordered_map<UUID, EntityData> data;
		static const Prefab GetFallbackPrefab() {
			Prefab p;
			p.data[0].materialIDs.push_back(ReservedUUID::NONE);
			return p;
		}
	};
}