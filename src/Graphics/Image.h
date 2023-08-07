namespace mcctp {
class Image {
public:
  Image(TexturePackResource res, bool load_immediately = true);
  Image(std::vector<uint8_t> &data, uint32_t width, uint32_t height, uint32_t channels);
  Image(unsigned char* data, uint32_t width, uint32_t height, uint32_t channels);
  ~Image();

  void Load(void);
  void Unload(void);

  void Bind(void)   const { glBindTexture(GL_TEXTURE_2D, m_RendererID); }
  void Unbind(void) const { glBindTexture(GL_TEXTURE_2D, 0); }

  bool IsLoaded(void) const { return m_Isloaded; }
  unsigned int GetWidth(void) const  { return m_Resource.Width; };
  unsigned int GetHeight(void) const { return m_Resource.Height; };
  std::string GetName(void) const { return m_Resource.Name; }
  char * GetOrigin(void) const { return m_Resource.Origin; }
  size_t GetOffset(void) const { return m_Resource.Offset; }
  size_t GetSize(void) const { return m_Resource.Size; }
  ResourceFormat GetFormat(void) const { return m_Resource.Format; }
  TexturePackType GetPackType(void) const { return m_Resource.PackType; }

  GLuint GetRendererID(void) const { return m_RendererID; };

private:
  GLuint CreateTexture(TexturePackResource res);
  GLuint CreateTexture(std::vector<uint8_t> &data, uint32_t width, uint32_t height,
                       uint32_t channels);
  GLuint CreateTexture(unsigned char* data, uint32_t width, uint32_t height,
                       uint32_t channels);

  void DeleteTexture(GLuint& renderer_id);

private:
  GLuint m_RendererID;
  TexturePackResource m_Resource;
  bool m_Isloaded;
};
}