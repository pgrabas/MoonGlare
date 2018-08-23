#pragma once

#include "iContext.h"

namespace MoonGlare::Renderer {

class iRendererFacade {
public:
    virtual ~iRendererFacade() { }

    virtual iContext* GetContext() = 0;

    virtual void SetConfiguration(Configuration::RuntimeConfiguration Configuration) = 0;
};

}
