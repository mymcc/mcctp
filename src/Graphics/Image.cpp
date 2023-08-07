#include "mcctppch.h"
#include "Image.h"

namespace mcctp {
Image::Image(TexturePackResource res, bool load_immediately) : m_RendererID(0), m_Resource(res), m_Isloaded(load_immediately) {
  if (load_immediately) {
    m_RendererID = CreateTexture(m_Resource);
  }
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
void Image::Load(void)
{
    if (!m_Isloaded) {
        m_RendererID = CreateTexture(m_Resource);
        m_Isloaded = true;
    }
}
void Image::Unload(void)
{
    DeleteTexture(m_RendererID);
    m_Isloaded = false;
}
GLuint Image::CreateTexture(TexturePackResource res) {
  //GLuint tex = mcctp::RenderDDSToTexture(res);


  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  if (res.Format != ResourceFormat::A8R8G8B8) {
      tex = mcctp::RenderDDSToTexture(res);
  }
  else {
    char* start = res.Origin + res.Offset;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.Width, res.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, start);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
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