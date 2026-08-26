#pragma once
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Render/Primitives/Primitives.h"
#include "Assets/Asset.h"
#include <unordered_map>
namespace Gaze {
	template <typename T>
	struct AssetStorage { //runtime
		std::unordered_map<UUID, std::weak_ptr<T>> storage;
		AssetType type;
		AssetStorage(AssetType t) : type(t) {}
		std::weak_ptr<T> Get(const UUID& id) const {
			auto it = storage.find(id);
			if (it != storage.end())
				return it->second;
			return std::weak_ptr<T>();
		}
		void Add(const UUID& id,std::weak_ptr<T> ptr) {
			if (!ptr.expired())
				storage.emplace(id, ptr);
		}
	};
	class AssetManager {
	public:
		template <typename T>
		static std::shared_ptr<T> Get(const UUID& id){
			auto& storage = GetStorage<T>();
			std::weak_ptr<T> existingAsset = storage.Get(id);
			std::shared_ptr<T> returnedAsset;
			
			if (returnedAsset = existingAsset.lock())
				return returnedAsset;
			else {
				Asset handle(id, storage.type);
				std::shared_ptr<T> loadedAsset = Load<T>(handle);
				storage.Add(id, std::weak_ptr(loadedAsset));
				return loadedAsset;
			}
		 }
	private:
		template <typename T> static AssetStorage<T>& GetStorage();
		template <> static AssetStorage<Mesh>& GetStorage() { return s_meshStorage; }
		template <> static AssetStorage<Shader>& GetStorage() { return s_shaderStorage; }
	private:
		template <typename T> static std::shared_ptr<T> Load(const Asset& asset);
		template <typename T> static std::shared_ptr<T> LoadReserved(const Asset& asset);

		template <> static std::shared_ptr<Mesh> Load(const Asset& asset) {
			if (asset.id.GetFlag() == 0)
				return LoadReserved<Mesh>(asset);
		}
		template <> static std::shared_ptr<Mesh> LoadReserved(const Asset& asset) {
			if (asset.id == ReservedUUID::TRIANGLE)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Triangle));
			if (asset.id == ReservedUUID::QUAD)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Quad));
			if (asset.id == ReservedUUID::CUBE)
				return std::make_shared<Mesh>(Primitives::LoadPrimitiveByType(PrimitiveType::Cube));
		}

		template <> static std::shared_ptr<Shader> Load(const Asset& asset) {
			if (asset.id.GetFlag() == 0)
				return LoadReserved<Shader>(asset);
		}
		template <> static std::shared_ptr<Shader> LoadReserved(const Asset& asset) {
			if (asset.id == ReservedUUID::DEFAULTSHADER)
				return std::make_shared<Shader>("Gaze/assets/shaders/DefaultShader.shader");
		}
	private:
		inline static AssetStorage<Mesh> s_meshStorage{ AssetType::Mesh };
		inline static AssetStorage<Shader> s_shaderStorage{ AssetType::Shader };
	};
}