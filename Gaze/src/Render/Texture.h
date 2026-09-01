#pragma once
#include <filesystem>
#include <memory>
namespace Gaze {
	class Texture {
	public:
		Texture(const std::filesystem::path& filepath);
		~Texture();
		void Bind(uint32_t slot);
	private:
		uint32_t m_textureID;
	};
}