/*
  * Generated by cppsrc.sh
  * On 2015-03-06  9:44:21,50
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include "nfGUI.h"
#include "Animation.h"

#include "Core/Engine.h"
#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>

namespace MoonGlare {
namespace GUI {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Animation);

Animation::Animation():
        BaseClass(),
        m_Speed(0.1f),
        m_StartFrame(0),
        m_EndFrame(0),
        m_FrameSpacing(0),
        m_FrameCount(1),
        m_DrawEnabled(false) {
}

Animation::~Animation() {
}

//----------------------------------------------------------------'

bool Animation::Load(const std::string &fileuri, unsigned StartFrame, unsigned FrameCount, math::uvec2 FrameStripCount, math::uvec2 Spacing, math::vec2 FrameSize, bool Uniform, const emath::fvec2 &ScreenSize) {
    m_StartFrame = StartFrame;
    m_EndFrame = m_StartFrame + FrameCount - 1;

    m_Name = fileuri;

    m_FrameCount = FrameStripCount;
    m_FrameSpacing = Spacing;

    if (FrameCount == 0) {
        FrameCount = 1;
        m_StartFrame = m_EndFrame = 0;
    }

    auto *e = Core::GetEngine();
    auto *rf = e->GetWorld()->GetRendererFacade();
    auto *resmgr = rf->GetResourceManager();

    auto matb = resmgr->GetMaterialManager().GetMaterialBuilder(m_Material, true);
    matb.SetDiffuseColor(emath::fvec4(1));
    matb.SetDiffuseMap(fileuri);
    m_TextureSize = emath::MathCast<math::fvec2>(resmgr->GetTextureResource().GetSize(matb.m_MaterialPtr->m_DiffuseMap));

    if (Uniform) {
        auto screen = emath::MathCast<math::fvec2>(ScreenSize);
        float Aspect = screen[0] / screen[1];
        FrameSize = m_TextureSize;
        FrameSize /= math::vec2(FrameStripCount);
        FrameSize /= screen;
        FrameSize.x *= Aspect;
        FrameSize *= 2.0f;
        m_FrameSize = FrameSize;
    } else {
        if (FrameCount == 1 && (FrameSize[0] == 0 || FrameSize[1] == 0)) {
            FrameSize = m_TextureSize;
        }
    }

    if (!GenerateFrames(FrameSize, FrameStripCount)) {
        AddLog(Error, "Unable to frames for animation!");
        return false;
    }

    GetRenderDevice()->RequestContextManip([this]() {
        m_DrawEnabled = true;
        AddLog(Debug, "Animation '" << this->GetName() << "' has been loaded");
    });

    return true;
}

bool Animation::GenerateFrames(math::vec2 FrameSize, math::vec2 FrameStripCount) {
    unsigned FrameCount = m_EndFrame - m_StartFrame + 1;
    m_FrameTable.reset(new Graphic::VAO[FrameCount]);

    math::vec2 texsize = m_TextureSize;

    math::vec2 fu = math::vec2(1.0f) / FrameStripCount;

    for (unsigned y = 0; y < m_FrameCount[1]; ++y)
    for (unsigned x = 0; x < m_FrameCount[0]; ++x) {
        unsigned frame = y * m_FrameCount[0] + x;
        if (frame > FrameCount)
            continue;

        Graphic::VertexVector Vertexes{
            Graphic::vec3(0, FrameSize[1], 0),
            Graphic::vec3(FrameSize[0], FrameSize[1], 0),
            Graphic::vec3(FrameSize[0], 0, 0),
            Graphic::vec3(0, 0, 0),
        };
        Graphic::NormalVector Normals;
        float w1 = fu[0] * (float)x;
        float h1 = fu[1] * (float)y;
        float w2 = w1 + fu[0];
        float h2 = h1 + fu[1];
        Graphic::TexCoordVector TexUV{
            Graphic::vec2(w1, h1),
            Graphic::vec2(w2, h1),
            Graphic::vec2(w2, h2),
            Graphic::vec2(w1, h2),
        };

        Graphic::IndexVector Index{ 0, 1, 2, 0, 2, 3, };
        m_FrameTable[frame].DelayInit(Vertexes, TexUV, Normals, Index);
    }

    return true;
}

//----------------------------------------------------------------

AnimationInstance Animation::CreateInstance() {
    AnimationInstance ai;
    ai.Data = shared_from_this();
    ai.Position = static_cast<float>(m_StartFrame);
    return ai;
}
 
void Animation::UpdateInstance(const Core::MoveConfig &conf, AnimationInstance &instance) {
    if (m_Speed > 0) {
        instance.Position += m_Speed * conf.TimeDelta;
        if (instance.Position > m_EndFrame) {
            auto delta = m_EndFrame - m_StartFrame;
            int mult = static_cast<int>(instance.Position / delta);
            instance.Position -= static_cast<float>(mult) * delta;
        } 
        //else
            //if (instance.Position < m_StartFrame) {
                //auto delta = m_EndFrame - m_StartFrame;
                //int mult = static_cast<int>(instance.Position / delta);
            //}
    }
}

const Graphic::VAO& Animation::GetFrameVAO(unsigned Frame) const {
    if (Frame > m_EndFrame)
        Frame = m_EndFrame;
    else
        if (Frame < m_StartFrame)
            Frame = m_StartFrame;
    return m_FrameTable[Frame];
}

} //namespace GUI 
} //namespace MoonGlare 
