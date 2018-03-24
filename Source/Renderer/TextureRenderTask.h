/*
  * Generated by cppsrc.sh
  * On 2017-02-11 20:30:17,81
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#include "nfRenderer.h"
#include "Commands/CommandQueue.h"

namespace MoonGlare::Renderer {

class alignas(16) TextureRenderTask final {
public:
 	TextureRenderTask();
 	~TextureRenderTask();

	bool Initialize();
	bool Finalize();

	void Begin();
	void End();

	void SetFrame(Frame *frame) { m_Frame = frame; }
	void SetTarget(TextureResourceHandle &handle, emath::ivec2 Size);

	Commands::CommandQueue& GetCommandQueue() { return m_CommandQueue; }
	template<typename CMD, typename ...ARGS>
	typename CMD::Argument* PushCommand(ARGS&& ...args) {
		return m_CommandQueue.PushCommand<CMD>(std::forward<ARGS>(args)...);
	}

private: 
	TextureResourceHandle m_TargetTexture;
	Device::FramebufferHandle m_Framebuffer;
    uint32_t padding;
	emath::ivec2 m_Size;
	Frame* m_Frame;
	void* padding2;

	Commands::CommandQueue m_CommandQueue;
};

static_assert((sizeof(TextureRenderTask) & 0xF) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer 
