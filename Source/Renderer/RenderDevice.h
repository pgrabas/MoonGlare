/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:46:28,36
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "nfRenderer.h"
#include "Configuration.Renderer.h"

#include "TextureRenderTask.h"

namespace MoonGlare::Renderer {

class RendererFacade;

class alignas(16) RenderDevice final {
	using ThisClass = RenderDevice;
	using Conf = Configuration::FrameBuffer;
public:
	bool Initialize(RendererFacade *renderer);
	bool Finalize();
	
	Frame* NextFrame();
	void Submit(Frame *frame);
	void ReleaseFrame(Frame *frame);
	Frame* PendingFrame();

	void Step();

	TextureRenderTask* AllocateTextureRenderTask() {
		RendererAssert(this);
		return m_UnusedTextureRender.pop(nullptr);
	}
private:
	std::array<mem::aligned_ptr<Frame>, Conf::Count> m_Frames;
	std::atomic<uint32_t> m_FreeFrameBuffers = 0;
	std::atomic<Frame*> m_PendingFrame = nullptr;

	Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit> m_UnusedTextureRender;
	std::array<TextureRenderTask, Configuration::TextureRenderTask::Limit> m_TextureRenderTask;

	DeclarePerformanceCounter(DroppedFrames);
	DeclarePerformanceCounter(FramesProcessed);

	RendererFacade *m_RendererFacade = nullptr;
	
	void ProcessFrame(Frame *frame);
};

} //namespace MoonGlare::Renderer
