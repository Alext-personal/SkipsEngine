#pragma once
#include "Core/UUID.h"
#include "Core/Log.h"
#include <string>
namespace Gaze {
	enum class AssetType {
		None,Mesh, Material, Shader,Texture
	};
	inline std::string AssetTypeToString(const AssetType& type) {
		switch (type) {
		case AssetType::Material :
			return "Material";
		case AssetType::Mesh:
			return "Mesh";
		case AssetType::Shader:
			return "Shader";
		case AssetType::Texture:
			return "Texture";
		}
	}
	inline AssetType StringToAssetType(const std::string& str) {
		if (str == "Material")
			return AssetType::Material;
		if (str == "Mesh")
			return AssetType::Mesh;
		if (str == "Shader")
			return AssetType::Shader;
		if (str == "Texture")
			return AssetType::Texture;
		ENGINE_ASSERT("INVALID STRING ASSETTYPE");
	}
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