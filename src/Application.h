#pragma once

#include "Layer/Layer.h"

namespace mcctp {
struct ApplicationSpecification {
  std::string Name = "mcctp-example";
  uint32_t Width = 1600;
  uint32_t Height = 900;
};

class Application {
public:
  Application(
      const ApplicationSpecification &applicationSpecification = ApplicationSpecification());
  ~Application();

  void Run();
  void Close();

  static Application *Instance();
  uint32_t &GetWidth() { return m_Specification.Width; }
  uint32_t &GetHeight() { return m_Specification.Height; }

  template <typename T> void PushLayer() {
    static_assert(std::is_base_of<mcctp::Layer, T>::value, "Pushed type is not subclass of Layer!");
    m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
  }

  void PushLayer(const std::shared_ptr<mcctp::Layer> &layer) {
    m_LayerStack.emplace_back(layer);
    layer->OnAttach();
  }

private:
  void Init();
  void Shutdown();

  void StartNewAppFrame();
  void RenderAppFrame();

  void CenterWindow();

private:
  HWND m_hHWND;
  WNDCLASSEXW m_hWC;
  ApplicationSpecification m_Specification;
  std::vector<std::shared_ptr<mcctp::Layer>> m_LayerStack;
  bool m_Running = false;
};
} // namespace mcctp
