#pragma once
#include <filesystem>
#include <memory>
class Texture {
	Texture(const std::filesystem::path& filepath);
	~Texture();
private:
	uint32_t m_textureID;
};