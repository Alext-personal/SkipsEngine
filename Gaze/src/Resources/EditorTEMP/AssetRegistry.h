#pragma once
#include "Resources/Asset.h"
namespace Gaze {
	class YAML::Emitter;
	class YAML::Node;
	struct IImportSettings {
		IImportSettings() = default;
		virtual ~IImportSettings() {}
		virtual YAML::Node Serialize() const = 0;
		virtual void DeSerialize(const YAML::Node& in) = 0;
		virtual std::unique_ptr<IImportSettings> Clone() = 0;
		virtual uint64_t GetHash() = 0;
	};
	struct TextureImportSettings : IImportSettings {
		uint32_t mipcount = 1;
		TextureImportSettings() :mipcount(1) {}
		TextureImportSettings(const YAML::Node in) {
			DeSerialize(in);
		}
		YAML::Node Serialize() const override;
		void DeSerialize(const YAML::Node& in) override;
		std::unique_ptr<IImportSettings> Clone() override;
		uint64_t GetHash() override;
	};
	struct AssetDependency {
		std::filesystem::path nodePath;
		UUID id;
		AssetType type;
	};
	struct MetaData {
		std::filesystem::path source;
		bool isStandalone;
		UUID generatedFrom;
		UUID id;
		uint64_t importHash;
		uint64_t snapshotHash = 0;
		AssetType assetType;
		std::vector<AssetDependency> generatedDependencies;
		std::unique_ptr<IImportSettings> importSettings;

		MetaData& operator=(const MetaData& other) {
			if (other.importSettings != nullptr)
				importSettings = other.importSettings->Clone();
			source = other.source;
			id = other.id;
			generatedFrom = other.generatedFrom;
			isStandalone = other.isStandalone;
			importHash = other.importHash;
			assetType = other.assetType;
			generatedDependencies = other.generatedDependencies;
			return *this;
		}
		MetaData() = default;
	};
	struct AssetRegistry {
		std::unordered_map<UUID, MetaData> storage;
		std::filesystem::path currentPath{ GAZE_SOURCE_ASSET_ROOT }; // hardcoded for testing purposes
		bool Has(const UUID& id) {
			auto it = storage.find(id);
			if (it != storage.end())
				return true;
			return false;
		}
	};
}
namespace YAML {
	template<>
	struct convert<Gaze::AssetDependency> {
		static Node encode(const Gaze::AssetDependency& rhs) {
			Node node;
			node["NodePath"] = rhs.nodePath.string();
			node["UUID"] = rhs.id.Get();
			node["Type"] = Gaze::AssetTypeToString(rhs.type);
			return node;
		}

		static bool decode(const Node& node, Gaze::AssetDependency& rhs) {
			if (!node.IsMap())
				return false;
			if (!node["NodePath"] || !node["UUID"] || !node["Type"])
				return false;
			Gaze::LOG_ERROR("SHIT STILL HERE WTF ");
			rhs.nodePath = std::filesystem::path(node["NodePath"].as<std::string>());
			rhs.id = node["UUID"].as<uint64_t>();
			rhs.type = Gaze::StringToAssetType(node["Type"].as<std::string>());
			return true;
		}
	};
}