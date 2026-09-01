#include "pch.h"
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace Gaze {
	Texture::Texture(const std::filesystem::path& filepath) {
		glCreateTextures(GL_TEXTURE_2D,1, &m_textureID);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int width, height, nrChannels;
		unsigned char* data = stbi_load(filepath.string().c_str(), &width, &height, &nrChannels, 4);
		if (!data) {
			LOG_ERROR("${} FAILED TO LOAD", filepath);
			//fallback code texture
			uint8_t pixel[4] = { 255, 0, 255, 255 };
			glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureStorage2D(m_textureID, 1, GL_RGBA8, 1, 1);
			glTextureSubImage2D(m_textureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		}
		else {
			glTextureStorage2D(m_textureID, 4, GL_RGBA8, width, height);
			glTextureSubImage2D(m_textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateTextureMipmap(m_textureID);
			stbi_image_free(data);
		}
	}
	void Texture::Bind(uint32_t slot) {
		glBindTextureUnit(slot, m_textureID); // TEMP WHICH SAMPLER2D UNIFORM DOES TEXTURE GO IN (0  = FIRST, AND SO ON )
	}
	Texture::~Texture() {
		glDeleteTextures(1, &m_textureID);
	}
}