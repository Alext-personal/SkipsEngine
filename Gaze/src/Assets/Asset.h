#pragma once
#include "Core/UUID.h"
namespace Gaze {
	enum class AssetType {
		Mesh, Material, Shader
	};
	template <typename T>
	struct AssetHandle {
		UUID id;
		std::shared_ptr<T> asset;
	};
	struct Asset {
		UUID id;
		AssetType type;
		Asset(const UUID& _id, const AssetType _type) :id(_id), type(_type){}
	};
}