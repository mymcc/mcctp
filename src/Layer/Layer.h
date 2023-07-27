#pragma once

namespace mcctp {
class Layer {
public:
  virtual ~Layer() = default;

  virtual void OnAttach() {}
  virtual void OnDetach() {}

  virtual void OnUpdate(float ts) {}
  virtual void OnUIRender() {}
};
} // namespace mcctp