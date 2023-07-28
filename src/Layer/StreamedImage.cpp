#include "mcctppch.h"
#include "StreamedImage.h"

mcctp::StreamedImage::StreamedImage(std::filesystem::path path) {
	std::wstring dst;
	dst.resize(MAX_PATH);
	::ExpandEnvironmentStringsW(path.wstring().c_str(), dst.data(), MAX_PATH);
	m_TemporaryDirectory = std::filesystem::absolute(dst);
}

void mcctp::StreamedImage::OnAttach() {
	auto path_prefix = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
	path_prefix += "/ui/texturepacks";
	std::cout << path_prefix.generic_string() << std::endl;
	//mcctp::Initialize(path_prefix, mcctp::TexturePackFlags::Emblems);
	mcctp::Initialize(path_prefix);
	// mcctp::MemoryMapTexturePacks();
	mcctp::MemoryMapAndIndexTexturePacks();
}

void mcctp::StreamedImage::OnUpdate(float ts) {}

void mcctp::StreamedImage::OnUIRender() {
  ImGui::Begin("StreamedImageViewer");
  if (ImGui::Button("Dump TexturePacks")) {
	  mcctp::DumpTexturePacks();
  }
  ImGui::End();
}