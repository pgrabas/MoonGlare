/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:34:50,23
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/
#pragma once

#include "Configuration.Renderer.h"
#include "FrameBuffer.h"

#include "Resources/AssetLoaderInterface.h"

namespace MoonGlare::Renderer {

class RendererFacade final {
public:
	RendererFacade();
 	~RendererFacade();

    bool Initialize(const ContextCreationInfo& ctxifo, Resources::AssetLoader *Assets);
    bool Finalize();

    /** Shall work on main thread; does not return until stopped */
    void Start();
    void Stop();

//	Context* CreateContext(const ContextCreationInfo& ctxifo);

	Context* GetContext() {
		RendererAssert(this);
		return m_Context.get();
	}
	RenderDevice* GetDevice() {
		RendererAssert(this);
		return m_Device.get();
	}
	Resources::ResourceManager* GetResourceManager() {
		RendererAssert(this);
		return m_ResourceManager.get();
	}

	ScriptApi* GetScriptApi();
private:
    bool m_CanWork = false;

	mem::aligned_ptr<RenderDevice> m_Device;
	mem::aligned_ptr<Context> m_Context;
	mem::aligned_ptr<Resources::ResourceManager> m_ResourceManager;
	std::unique_ptr<ScriptApi> m_ScriptApi;
};

} //namespace MoonGlare::Renderer
