#include "mcctppch.h"
#include "Image.h"

namespace mcctp {
Image::Image(TexturePackResource res) : m_RendererID(0), m_Resource(res) {
  m_RendererID = CreateTexture(m_Resource);
}
Image::Image(std::vector<uint8_t> &data, uint32_t width, uint32_t height, uint32_t channels)
    : m_RendererID(0) 
{
  m_RendererID = CreateTexture(data, width, height, channels);
}
Image::Image(unsigned char* data, uint32_t width, uint32_t height, uint32_t channels)
    : m_RendererID(0) {
  m_RendererID = CreateTexture(data, width, height, channels);
}
Image::~Image() { 
	DeleteTexture(m_RendererID); 
}
GLuint Image::CreateTexture(TexturePackResource res) {
  GLuint tex = mcctp::RenderDDSToTexture(res);
  //GLuint tex;
  //glGenTextures(1, &tex);
  //glBindTexture(GL_TEXTURE_2D, tex);
  //
  //char *start = res.Origin + res.Offset;
  //
  //if (res.Format != ResourceFormat::A8R8G8B8) {
  //  uint16_t decompress_from = 0;
  //  if (res.Format == ResourceFormat::DXT1) {
  //    decompress_from = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
  //  } else if (res.Format == ResourceFormat::DXT3) {
  //    decompress_from = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  //  } else if (res.Format == ResourceFormat::DXT5) {
  //    decompress_from = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  //  } else {
  //    // Handle unsupported formats
  //  }
  //
  //  glCompressedTexImage2D(GL_TEXTURE_2D, 0, decompress_from, res.Width, res.Height, 0, res.Size,
  //                         start);
  //
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //  glGenerateMipmap(GL_TEXTURE_2D);
  //} else {
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.Width, res.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, start);
  //
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //}
  //
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex; 
}
GLuint Image::CreateTexture(std::vector<uint8_t> &data, uint32_t width, uint32_t height,
                            uint32_t channels) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}
GLuint Image::CreateTexture(unsigned char* data, uint32_t width, uint32_t height,
                            uint32_t channels) {
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}
void Image::DeleteTexture(GLuint& renderer_id) { 
	glDeleteTextures(1, &renderer_id); 
}
} // namespace mcctp