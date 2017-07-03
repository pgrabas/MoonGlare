/*
  * Generated by cppsrc.sh
  * On 2015-03-06  9:44:21,50
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>

#define NEED_MATERIAL_BUILDER     
#define NEED_VAO_BUILDER     

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

Animation::Animation() :
    m_Speed(0.1f),
    m_StartFrame(0),
    m_EndFrame(0),
    m_FrameSpacing(0),
    m_FrameCount(1),
    m_DrawEnabled(false) {}

Animation::~Animation() {}

//----------------------------------------------------------------'

bool Animation::Load(const std::string &fileuri, unsigned StartFrame,
    unsigned FrameCount, math::uvec2 FrameStripCount, math::uvec2 Spacing,
    math::vec2 FrameSize, bool Uniform, const emath::fvec2 &ScreenSize) {
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

    auto matb = resmgr->GetMaterialManager().GetMaterialBuilder(material, true);
    matb.SetDiffuseColor(emath::fvec4(1));
    matb.SetDiffuseMap(fileuri, true);
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
    }
    else {
        if (FrameCount == 1 && (FrameSize[0] == 0 || FrameSize[1] == 0)) {
            FrameSize = m_TextureSize;
        }
    }

    if (!GenerateFrames(FrameSize, FrameStripCount)) {
        AddLog(Error, "Unable to frames for animation!");
        return false;
    }

    return true;
}

bool Animation::GenerateFrames(math::vec2 FrameSize, math::vec2 FrameStripCount) {
    auto *e = Core::GetEngine();
    auto *rf = e->GetWorld()->GetRendererFacade();

    unsigned FrameCount = m_EndFrame - m_StartFrame + 1;
    frames.clear();
    frames.resize(FrameCount, {});

    //FIXME: ugly!
    rf->GetAsyncLoader()->QueueTask(std::make_shared < Renderer::FunctionalAsyncTask>(
        [this, rf, FrameSize, FrameStripCount, FrameCount](Renderer::ResourceLoadStorage &storage) {

        math::vec2 texsize = m_TextureSize;
        math::vec2 fu = math::vec2(1.0f) / FrameStripCount;

        std::vector<glm::fvec3> Vertexes;
        std::vector<glm::fvec2> UVs;

        Vertexes.reserve(FrameCount * 4);
        UVs.reserve(FrameCount * 4);
        UVs.reserve(FrameCount * 4);

        for (unsigned y = 0; y < m_FrameCount[1]; ++y)
            for (unsigned x = 0; x < m_FrameCount[0]; ++x) {
                unsigned frame = y * m_FrameCount[0] + x;
                if (frame > FrameCount)
                    continue;

                auto &f = frames[frame];

                f.baseIndex = 0;
                f.baseVertex = (uint16_t)Vertexes.size();
                f.indexElementType = GL_UNSIGNED_BYTE;
                f.numIndices = 6;

                Vertexes.push_back(glm::fvec3(0, FrameSize[1], 0));
                Vertexes.push_back(glm::fvec3(FrameSize[0], FrameSize[1], 0));
                Vertexes.push_back(glm::fvec3(FrameSize[0], 0, 0));
                Vertexes.push_back(glm::fvec3(0, 0, 0));

                Graphic::NormalVector Normals;
                float w1 = fu[0] * (float)x;
                float h1 = fu[1] * (float)y;
                float w2 = w1 + fu[0];
                float h2 = h1 + fu[1];

                UVs.push_back(glm::fvec2(w1, h1));
                UVs.push_back(glm::fvec2(w2, h1));
                UVs.push_back(glm::fvec2(w2, h2));
                UVs.push_back(glm::fvec2(w1, h2));
            }


        {
            auto &m = storage.m_Memory.m_Allocator;
            auto &q = storage.m_Queue;

            using ichannels = Renderer::Configuration::VAO::InputChannels;

            auto vaob = rf->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vaoHandle, true);
            vaob.BeginDataChange();

            vaob.CreateChannel(ichannels::Vertex);
            vaob.SetChannelData<float, 3>(ichannels::Vertex, (const float*)m.Clone(Vertexes), Vertexes.size());

            vaob.CreateChannel(ichannels::Texture0);
            vaob.SetChannelData<float, 2>(ichannels::Texture0, (const float*)m.Clone(UVs), UVs.size());

            vaob.CreateChannel(ichannels::Index);
            static constexpr std::array<uint8_t, 6> IndexTable = { 0, 1, 2, 0, 2, 3, };
            vaob.SetIndex(ichannels::Index, IndexTable);

            vaob.EndDataChange();
            vaob.UnBindVAO();
        }
        m_DrawEnabled = true;
    }));

    return true;
}

} //namespace GUI 
} //namespace MoonGlare 
