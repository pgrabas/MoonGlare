#pragma once

namespace MoonGlare {
namespace Core {

    struct MoveConfig;

    class HandleTable;
    class InputProcessor;
    class Hooks;	   

    class Engine;

    struct RuntimeConfiguration;

}//namespace Core
}//namespace MoonGlare

namespace Core {
    using ::MoonGlare::Core::MoveConfig;
}

namespace Graphic {
namespace Light {
    struct LightConfigurationVector;
}
}

namespace MoonGlare {
namespace Renderer {

struct RenderInput;

}
}

#include "EntityManager.h"

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"

#include "Component/nfComponent.h"

namespace MoonGlare {
namespace Core {

const Version::Info& GetMoonGlareEngineVersion();

class TextProcessor;

struct iCustomDraw {
    virtual void DefferedDraw(Graphic::cRenderDevice& dev) { }
protected:
    virtual ~iCustomDraw() { }
};

struct MoveConfig { 
    float TimeDelta;

    emath::fvec2 m_ScreenSize;

    Renderer::Frame *m_BufferFrame;

    mutable Renderer::VirtualCamera *Camera = nullptr;
    
    //mutable mem::aligned_ptr<Renderer::RenderInput> m_RenderInput;
    Graphic::Dereferred::DefferedSink *deferredSink;

    bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare
