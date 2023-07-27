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
	mcctp::Initialize("F:\\SteamLibrary\\steamapps\\common\\Halo The Master Chief Collection\\Data\\ui\\texturepacks");
	//mcctp::MemoryMapTexturePacks();
	mcctp::MemoryMapAndIndexTexturePacks();
}

void mcctp::StreamedImage::OnUpdate(float ts)
{
}

void mcctp::StreamedImage::OnUIRender()
{
	ImGui::Begin("StreamedImageViewer");
	ImGui::End();
}