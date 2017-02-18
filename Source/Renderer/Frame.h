/*
  * Generated by cppsrc.sh
  * On 2017-02-10 15:35:25,52
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueueLayers.h"

namespace MoonGlare::Renderer {

class alignas(16) Frame final {
	using Conf = Configuration::FrameBuffer;
public:
	using TextureRenderQueue = Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit>;

	template<typename T>
	using ByteArray = Space::Memory::StaticMemory<T, Conf::MemorySize>;
	using Allocator_t = Space::Memory::StackAllocator<ByteArray>;
	using CommandLayers = Commands::CommandQueueLayers<Conf::Layers>;

	CommandLayers& GetCommandLayers() {
		RendererAssert(this);
		return m_CommandLayers;
	}
	CommandLayers::Queue& GetControllCommandQueue() {
		RendererAssert(this);
		return m_CommandLayers.Get<Conf::Layers::Controll>();
	}
	TextureRenderQueue& GetTextureRenderQueue() { 
		RendererAssert(this);
		return m_QueuedTextureRender; 
	}
	Allocator_t& GetMemory() { 
		RendererAssert(this);
		return m_Memory;
	}

	void BeginFrame();
	void EndFrame();

	bool Submit(TextureRenderTask *trt);

	bool Initialize(uint8_t BufferIndex, RenderDevice *device, Resources::ResourceManager *ResMgr);
	bool Finalize();

	uint8_t Index() const { return m_BufferIndex; }
	RenderDevice* GetDevice() const { return m_RenderDevice; }
	Resources::ResourceManager* GetResourceManager() const { return m_ResourceManager; }
private: 
	uint8_t m_BufferIndex;
	uint8_t padding8[3];
	RenderDevice *m_RenderDevice;
	Resources::ResourceManager *m_ResourceManager;
	void *paddingptr;

	CommandLayers m_CommandLayers;

	TextureRenderQueue m_QueuedTextureRender;
	Allocator_t m_Memory;
};

static_assert((sizeof(Frame) % 16) == 0, "Invalid size!");
//static_assert(std::is_pod<Frame>::value, "Must be a pod!");

} //namespace MoonGlare::Renderer 
