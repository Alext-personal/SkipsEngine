#include "pch.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include <yaml-cpp/yaml.h>
#include <xxhash.h>
namespace Gaze {
	YAML::Node TextureImportSettings::Serialize() const
	{
		YAML::Node node;
		node["MipCount"] = mipcount;
		return node;
	}
	void TextureImportSettings::DeSerialize(const YAML::Node& in)
	{
		mipcount = in["MipCount"].as<uint32_t>();
	}
	std::unique_ptr<IImportSettings> TextureImportSettings::Clone()
	{
		return std::make_unique<TextureImportSettings>(*this);
	}
	uint64_t TextureImportSettings::GetHash()
	{
		return XXH3_64bits(&mipcount,1);
	}
}