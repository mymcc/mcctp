namespace mcctp {
class Image {
public:
  Image(TexturePackResource res);
  ~Image();

  unsigned int GetWidth() const  { return m_Resource.Width; };
  unsigned int GetHeight() const { return m_Resource.Height; };
  GLuint GetRendererID() const { return m_RendererID; };

private:
  GLuint CreateTexture(TexturePackResource res);
  void DeleteTexture(GLuint& renderer_id);

private:
  GLuint m_RendererID;
  TexturePackResource m_Resource;
};
}