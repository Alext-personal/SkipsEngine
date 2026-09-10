#include "pch.h"
#include "Texture.h"
namespace Gaze {
	inline GLenum PixelDataFormatToGlFormat(PixelDataFormat pixel)
	{
		switch (pixel)
		{
		case PixelDataFormat::R8: return GL_RED;
		case PixelDataFormat::R16: return GL_RED;
		case PixelDataFormat::R32F: return GL_RED;
		case PixelDataFormat::RG8: return GL_RG;
		case PixelDataFormat::RG16: return GL_RG;
		case PixelDataFormat::RG32F: return GL_RG;
		case PixelDataFormat::RGB8: return GL_RGB;
		case PixelDataFormat::RGB16: return GL_RGB;
		case PixelDataFormat::RGB32F: return GL_RGB;
		case PixelDataFormat::RGBA8: return GL_RGBA;
		case PixelDataFormat::RGBA16: return GL_RGBA;
		case PixelDataFormat::RGBA32F: return GL_RGBA;
		}

		return GL_RGBA;
	}
	inline GLenum PixelDataFormatToSizedGlFormat(PixelDataFormat format) {
		switch (format) {
			case PixelDataFormat::R8:      return GL_R8;
			case PixelDataFormat::R16:     return GL_R16;
			case PixelDataFormat::R32F:    return GL_R32F;
			case PixelDataFormat::RG8:     return GL_RG8;
			case PixelDataFormat::RG16:    return GL_RG16;
			case PixelDataFormat::RG32F:   return GL_RG32F;
			case PixelDataFormat::RGB8:    return GL_RGB8;
			case PixelDataFormat::RGB16:   return GL_RGB16;
			case PixelDataFormat::RGB32F:  return GL_RGB32F;
			case PixelDataFormat::RGBA8:   return GL_RGBA8;
			case PixelDataFormat::RGBA16:  return GL_RGBA16;
			case PixelDataFormat::RGBA32F: return GL_RGBA32F;
		}
	}
	inline GLenum PixelDataFormatToGlType(PixelDataFormat pixel)
	{	
		switch (pixel)
		{
			case PixelDataFormat::R8:
				return GL_UNSIGNED_BYTE;
			case PixelDataFormat::RG8:
				return GL_UNSIGNED_BYTE;
			case PixelDataFormat::RGB8:
				return GL_UNSIGNED_BYTE;
			case PixelDataFormat::RGBA8:
				return GL_UNSIGNED_BYTE;

			case PixelDataFormat::R16:
				return GL_UNSIGNED_SHORT;
			case PixelDataFormat::RG16:
				return GL_UNSIGNED_SHORT;
			case PixelDataFormat::RGB16:
				return GL_UNSIGNED_SHORT;
			case PixelDataFormat::RGBA16:
				return GL_UNSIGNED_SHORT;

			case PixelDataFormat::R32F:
				return GL_FLOAT;
			case PixelDataFormat::RG32F:
				return GL_FLOAT;
			case PixelDataFormat::RGB32F:
				return GL_FLOAT;
			case PixelDataFormat::RGBA32F:
				return GL_FLOAT;
		}

		return GL_UNSIGNED_BYTE;
	}
	Texture::Texture(const TextureData& data) {
		glCreateTextures(GL_TEXTURE_2D,1, &m_textureID);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		TextureData newData = data;
		if (data.data.empty()) {
			LOG_ERROR("FAILED TO LOAD TEXTURE, reverting to fallback");
			newData = Texture::GetFallbackTexture();
		}
		else {
			glTextureStorage2D(m_textureID, data.mipCount, PixelDataFormatToSizedGlFormat(data.format), data.width, data.height);
			for (uint32_t index = 0; index < data.mipCount; ++index) {
				glTextureSubImage2D(m_textureID, index, 0, 0, data.mips[index].width, data.mips[index].height , PixelDataFormatToGlFormat(data.format), PixelDataFormatToGlType(data.format), data.data.data() + data.mips[index].offset);
			}
		}
	}
	void Texture::Bind(uint32_t slot) {
		glBindTextureUnit(slot, m_textureID); // TEMP WHICH SAMPLER2D UNIFORM DOES TEXTURE GO IN (0  = FIRST, AND SO ON )
	}
	const TextureData& Texture::GetFallbackTexture() {
		static const TextureData fallback = [] {
			TextureData data;
			std::vector<uint8_t> fallbackPixels =  {
				255, 0, 255, 255,   // pink
				0, 0,   0, 255,   // black
				0, 0,   0, 255,   // black
				255, 0, 255, 255 // pink
			};
			data.data = fallbackPixels;
			data.format = PixelDataFormat::RGBA8;
			data.width = 2;
			data.height = 2;
			data.mipCount = 1;
			data.mips.push_back({ 2,2,0, });
			return data;
			}();
		return fallback;
	}
	Texture::~Texture() {
		glDeleteTextures(1, &m_textureID);
	}
}