#pragma once
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Render/Primitives/Primitives.h"
#include "Core/UUID.h"
#include <unordered_map>
namespace Gaze {
	enum class AssetType {
		Mesh, Material, Shader
	};
	struct MetaData {
		uint32_t version = 0;
		AssetType type;
		std::string filepath;
	};
	template <typename T>
	struct AssetStorage {
		std::unordered_map<UUID, std::weak_ptr<T>> storage;
		AssetType type;
		AssetStorage(AssetType t) : type(t) {}
		std::weak_ptr<T> Get(const UUID& id) const {
			auto it = storage.find(id);
			if (it != storage.end())
				return it->second;
			return nullptr;
		}
	};
	class AssetManager {
	public:
	private:
		inline static AssetStorage<Mesh> m_meshStorage{ AssetType::Mesh };
		inline static AssetStorage<Shader> m_shaderStorage{ AssetType::Shader };
	};
}