#include "pch.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include <yaml-cpp/yaml.h>
namespace Gaze {
	YAML::Node TextureImportSettings::Serialize() const
	{
		return YAML::Node();
	}
	void TextureImportSettings::DeSerialize(const YAML::Node& in)
	{
	}
	std::unique_ptr<IImportSettings> TextureImportSettings::Clone()
	{
		return std::make_unique<TextureImportSettings>(*this);
	}
	uint64_t TextureImportSettings::GetHash()
	{
		return 0;
	}
}