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
		uint32_t mipcount;
		TextureImportSettings() :mipcount(1) {}
		TextureImportSettings(const YAML::Node in) {
			DeSerialize(in);
		}
		YAML::Node Serialize() const override;
		void DeSerialize(const YAML::Node& in) override;
		std::unique_ptr<IImportSettings> Clone() override;
		uint64_t GetHash() override;
	};
	struct MetaData {
		std::filesystem::path source;
		UUID id;
		uint64_t importHash;
		AssetType assetType;
		std::vector<UUID> generatedDependencies;
		std::unique_ptr<IImportSettings> importSettings;

		MetaData& operator=(const MetaData& other) {
			importSettings = other.importSettings->Clone();
			source = other.source;
			id = other.id;
			importHash = other.importHash;
			assetType = other.assetType;
			generatedDependencies = other.generatedDependencies;
			return *this;
		}
		MetaData() = default;
	};
	struct AssetRegistry {
		std::unordered_map<UUID, MetaData> storage;
		bool Has(const UUID& id) {
			auto it = storage.find(id);
			if (it != storage.end())
				return true;
			return false;
		}
	};
}