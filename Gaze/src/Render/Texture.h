#pragma once
#include <filesystem>
#include <memory>
namespace Gaze {
	enum class PixelDataFormat {
		R8,
		RG8,
		RGB8,
		RGBA8,

		R16,
		RG16,
		RGB16,
		RGBA16,

		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
	};
	struct MipData{
		uint32_t width, height, offset, byteSize;
	};
	struct TextureData {
		uint32_t width, height;
		uint8_t mipCount;
		std::vector<MipData> mips;
		PixelDataFormat format;
		std::vector<uint8_t> data;
	};
	class Texture {
	public:
		Texture(const TextureData& data = {});
		~Texture();
		static const TextureData& GetFallbackTexture();
		void Bind(uint32_t slot);
	private:
		uint32_t m_textureID;
	};
}