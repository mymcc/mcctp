namespace mcctp {
class Image {
public:
  Image(TexturePackResource res);
  ~Image();

  void Bind()   const { glBindTexture(GL_TEXTURE_2D, m_RendererID); }
  void Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

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