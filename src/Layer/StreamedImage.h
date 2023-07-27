#pragma once
#include "Layer.h"

namespace mcctp
{
	class StreamedImage : public Layer
	{
	public:
		StreamedImage(std::filesystem::path path = L"%AppData%\\..\\LocalLow\\MCC\\Temporary");
		virtual void OnAttach() override;
		virtual void OnUpdate(float ts);
		virtual void OnUIRender() override;
	private:
	private:
		std::filesystem::path m_TemporaryDirectory;
	};
}