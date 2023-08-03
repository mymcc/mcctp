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
  void DoMainMenuBar(void);
  void DoInstanceConfiguration(void);
  void DoTexturePackDumper(void);
  void DoTextureViewer(void);
  void DoTextureInfo(void);

private:
  mcctp::DumpFormatFlags m_DumpFormatFlag;
  mcctp::DumpCompressionFlags m_CompressionFlag;
  std::filesystem::path m_TexturePacksPath;
  std::vector<std::shared_ptr<Image>> m_TextureThumbnails;
  std::shared_ptr<Image> m_AppIcon;
  bool m_MaintainAspectRatio = true;
  unsigned int selected_thumbnail = 0;
  bool show_demo = false;
  mcctp::TexturePackFlags selected_texture_pack = mcctp::TexturePackFlags::None;
};
} // namespace mcctp