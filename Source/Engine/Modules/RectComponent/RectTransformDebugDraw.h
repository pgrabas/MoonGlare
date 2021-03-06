#pragma once

namespace MoonGlare::GUI::Component {

class RectTransformComponent::RectTransformDebugDraw {
public:
    RectTransformDebugDraw();
    ~RectTransformDebugDraw();

    void DebugDraw(const Core::MoveConfig &conf, RectTransformComponent *Component);
private:
    bool ready = false;
    Renderer::ShaderResourceHandleBase shaderHandle;
};

} //namespace MoonGlare::GUI::Component 
