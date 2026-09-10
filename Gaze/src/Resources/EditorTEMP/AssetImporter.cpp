#include "pch.h"
#include "Resources/EditorTEMP/AssetImporter.h"
#include "Resources/EditorTEMP/AssetRegistry.h"
#include "Resources/SerializationHelpers.h"
#include "Core/Helpers.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace Gaze {
	std::filesystem::path AssetImporter::ImportModel(const UUID& id)
	{
		return std::filesystem::path();
	}
	std::filesystem::path AssetImporter::ImportTexture(const UUID& id,TextureImportSettings* settings)
	{
		TextureFileHeader header;
		std::filesystem::path filepath = m_registry.storage[id].source;
		std::string srcPath = filepath.string();
		if (!std::filesystem::exists(filepath) || filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist",filepath);
			return std::filesystem::path();
		}
		int width, height, nrChannels;
		unsigned char* data = stbi_load(srcPath.c_str(), &width, &height, &nrChannels, 0);
		if (!data || !settings || settings->mipcount <= 0 )
		{
			LOG_ERROR("FAILED TO IMPORT : ${} ", srcPath);
			return std::filesystem::path();
		}
		std::vector<unsigned char> databuffer;
		uint32_t totalSize = width * height * nrChannels;
		int w = width, h = height;
		for (int i = 1; i < settings->mipcount; ++i) {
			w = (w >> 1) > 1 ? (w >> 1) : 1;
			h = (h >> 1) > 1 ? (h >> 1) : 1;
			totalSize += w * h * nrChannels;
		}
		databuffer.resize(totalSize);
		memcpy(databuffer.data(), data, width*height*nrChannels);
		STBI_FREE(data);
		std::vector<MipDataPacked> mips;
		mips.resize(settings->mipcount);
		header.width = width;
		header.height = height;
		mips[0] = { header.width,header.height,0, header.width * header.height * nrChannels };
		header.mipCount = settings->mipcount;
		memcpy(header.validation, "Texture",8);
		stbir_pixel_layout layout;
		switch (nrChannels)
		{
		case 1:
			header.format = PixelDataFormat::R8;
			layout = STBIR_1CHANNEL;
			break;
		case 2:
			header.format = PixelDataFormat::RG8;
			layout = STBIR_2CHANNEL;
			break;
		case 3:
			header.format = PixelDataFormat::RGB8;
			layout = STBIR_RGB;
			break;
		case 4:
			header.format = PixelDataFormat::RGBA8;
			layout = STBIR_RGBA;
			break;
		default:
			LOG_ERROR("INVALID NUMBER OF CHANNELS FOR ${}", srcPath);
			return std::filesystem::path();
			break;
		}
		for (int i = 1; i < header.mipCount; ++i) {
			int mipW = (width >> 1) > 1 ? (width >> 1) : 1;
			int mipH = (height >> 1) > 1 ? (height >> 1) : 1;
			mips[i].width = mipW;
			mips[i].height = mipH;
			mips[i].offset = mips[i - 1].offset + mips[i - 1].byteSize;
			mips[i].byteSize = mipW * mipH * nrChannels;
			stbir_resize_uint8_linear(databuffer.data() + mips[i-1].offset , width, height, 0,
				databuffer.data() + mips[i].offset , mipW, mipH, 0,
				layout);
			width = mipW;
			height = mipH;
		}
		header.dataSize = totalSize;
		std::filesystem::path outputPath = GetCurrentPath() / "Library" / "Client" / "Textures" / id.ToString();
		outputPath += ".gtex";
		std::filesystem::create_directories(outputPath.parent_path());
		std::ofstream file(outputPath, std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to create file ${} ", outputPath);
			return std::filesystem::path();
		}
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		file.write(reinterpret_cast<const char*>(mips.data()), header.mipCount * sizeof(MipDataPacked));
		file.write(reinterpret_cast<const char*>(databuffer.data()), header.dataSize);
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportShader(const UUID& id)
	{
		std::vector<std::pair<std::string, ShaderType>>parsedShaders;
		std::string currentShaderCode;
		ShaderType currentShaderType = ShaderType::None;
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!std::filesystem::exists(filepath) || filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist",filepath);
			return std::filesystem::path();
		}
		std::stringstream shadersSource(DumpFileToString(filepath));
		std::string line;
		while (std::getline(shadersSource, line)) {
			std::stringstream identifierLine(line);
			std::string identifier;
			identifierLine >> identifier;
			if (identifier == "#type")
			{
				ShaderType newShaderType = ShaderType::None;
				std::string type;
				identifierLine >> type;
				if (type == "Vertex")
					newShaderType = ShaderType::Vertex;
				if (type == "Fragment")
					newShaderType = ShaderType::Fragment;

				if (newShaderType == ShaderType::None) {
					LOG_ERROR("INVALID SHADER TYPE, CHECK SHADER SOURCE CODE");
					return std::filesystem::path();
				}
				if (newShaderType != ShaderType::None)
				{
					parsedShaders.push_back({ currentShaderCode,currentShaderType });
					currentShaderCode.clear();
				}
				currentShaderType = newShaderType;
			}
			else {
				currentShaderCode += line + "\n";
			}
		}
		if (currentShaderType != ShaderType::None) {
			parsedShaders.push_back({ currentShaderCode,currentShaderType });
		}
		std::filesystem::path outputPath = GetCurrentPath() / "Library" / "Client" / "Shaders" / id.ToString();
		outputPath += ".gs";
		ShaderFileHeader header;
		header.shaderCount = parsedShaders.size();
		memcpy(&header.validation, "Shader",7);
		std::filesystem::create_directories(outputPath.parent_path());
		std::ofstream file(outputPath, std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to create file ${} ", outputPath);
			return std::filesystem::path();
		}
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
		for (auto& [shader, type] : parsedShaders) {
			ShaderDataPacked data;
			data.size = shader.size();
			data.type = type;
			file.write(reinterpret_cast<const char*>(&data), sizeof(data));
			file.write(reinterpret_cast<const char*>(shader.data()), data.size);
		}
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportMaterial(const UUID& id)
	{
		std::filesystem::path filepath = m_registry.storage[id].source;
		if (!std::filesystem::exists(filepath) || filepath.has_filename())
		{
			LOG_ERROR("${} Does not exist", filepath);
			return std::filesystem::path();
		}
		std::filesystem::path outputPath = GetCurrentPath() / "Library" / "Client" / "Materials" / id.ToString();
		outputPath += ".gmat";

		// check for missing textures / shaders
		YAML::Node file;
		try {
			file = YAML::LoadFile(filepath.string());
		}
		catch (const YAML::Exception& e) {
			LOG_ERROR("${}  -file failed to open : ${} ", filepath, e.what());
			return std::filesystem::path();
		}
		if (!file["Shader"] || !file["Texture"] || !file["Tint"])
		{
			LOG_ERROR("${} INVALID MATERIAL FILE FORMAT, SHADER OR ALBEDO MISSING", filepath);
			return std::filesystem::path();
		}
		UUID shader = UUID(file["Shader"].as<uint64_t>(), true);
		UUID albedo = UUID(file["Texture"].as<uint64_t>(), true);
		if (!m_registry.Has(shader))
			LOG_ERROR("SHADER REFERENCED IN ${} NOT FOUND", filepath);
		if (!m_registry.Has(albedo))
			LOG_ERROR("ALBEDO REFERENCED IN ${} NOT FOUND", filepath);

		std::filesystem::create_directories(outputPath.parent_path());
		std::filesystem::copy(filepath, outputPath,std::filesystem::copy_options::overwrite_existing);
		return outputPath;
	}
	std::filesystem::path AssetImporter::ImportPrefab(const UUID& id)
	{
		return std::filesystem::path();	
	}
}