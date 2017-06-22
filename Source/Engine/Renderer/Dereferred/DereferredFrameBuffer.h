/*
    Generated by cppsrc.sh
    On 2015-01-18  9:24:36,67
    by Paweu
*/

#pragma once
#ifndef DereferredFrameBuffer_H
#define DereferredFrameBuffer_H

namespace Graphic {
namespace Dereferred {

class DereferredFrameBuffer {
public:
    DereferredFrameBuffer();
    ~DereferredFrameBuffer();

    struct Buffers {
        enum {
            Position	= SamplerIndex::Position,
            Diffuse		= SamplerIndex::Diffuse,
            Normal		= SamplerIndex::Normal,
            MaxValue,
        };
    };

    void Free();
    bool Reset(const emath::fvec2 &ScreenSize);

    void BeginFrame();

    void BeginGeometryPass();

    //void SetReadBuffer(unsigned b) {
        //glReadBuffer(GL_COLOR_ATTACHMENT0 + b);
    //}

    void BeginLightingPass();
    void BeginFinalPass();
    GLuint m_Textures[Buffers::MaxValue];
    GLuint m_DepthTexture = 0;
    GLuint m_FinalTexture = 0;

    void Bind() const {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffer);
    }
    static void UnBind() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    GLuint m_FrameBuffer = 0;
private:
    bool FreeFrameBuffer();
    bool NewFrameBuffer();
    bool FinishFrameBuffer();
};

} //namespace Dereferred 
} //namespace Graphic 

#endif

