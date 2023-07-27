#include "mcctppch.h"
#include "StreamedImage.h"

mcctp::StreamedImage::StreamedImage(std::filesystem::path path)
{
	std::wstring dst; dst.resize(MAX_PATH);
	::ExpandEnvironmentStringsW(path.wstring().c_str(), dst.data(), MAX_PATH);
	m_TemporaryDirectory = std::filesystem::absolute(dst);
}

void mcctp::StreamedImage::OnAttach()
{

}

void mcctp::StreamedImage::OnUpdate(float ts)
{
}

void mcctp::StreamedImage::OnUIRender()
{
	ImGui::Begin("StreamedImageViewer");
	ImGui::End();
}