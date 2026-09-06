#pragma once
#include "Core/UUID.h"
#include <vector>
namespace Gaze {
	struct Prefab {
		std::vector<UUID> meshIDs;
		std::vector<UUID> materialIDs;
		std::vector<uint32_t> parents; // parents[index in meshIDs]
	};
}