#pragma once
#include "Layer.h"
#include "Graphics/Image.h"

namespace mcctp {
class StreamedImage : public Layer {
public:
  StreamedImage(std::filesystem::path path = L"%AppData%\\..\\LocalLow\\MCC\\Temporary");
  virtual void OnAttach() override;
  virtual void OnUpdate(float ts);
  virtual void OnUIRender() override;

private:
  void DoInstanceConfiguration(void);
  void DoTexturePackDumper(void);
  void DoTextureViewer(void);

private:
  mcctp::DumpFormatFlags m_DumpFormatFlag;
  mcctp::DumpCompressionFlags m_CompressionFlag;
  std::filesystem::path m_TexturePacksPath;
  std::vector<std::shared_ptr<Image>> m_TextureThumbnails;
};
} // namespace mcctp