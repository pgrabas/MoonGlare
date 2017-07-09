/*
    Generated by cppsrc.sh
    On 2014-12-29 22:15:13,85
    by Paweu
*/
#include <pch.h>

#define NEED_VAO_BUILDER

#include <MoonGlare.h>

#include "DebugDrawer.h"

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/ShaderCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Engine/Renderer/Dereferred/DereferredPipeline.h>

namespace Physics {

struct BtDebugDrawShaderDescriptor {
    enum class InLayout {
        Position,
        Color,
    };
    enum class OutLayout {
        FragColor,
    };
    enum class Uniform {
        CameraMatrix,
        MaxValue,
    };
    enum class Sampler {
        MaxValue,
    };

    constexpr static const char* GetName(Uniform u) {
        switch (u) {
        case Uniform::CameraMatrix: return "CameraMatrix";
        default: return nullptr;
        }
    }
    constexpr static const char* GetSamplerName(Sampler s) {
        //switch (s) {
        //default: 
            return nullptr;
        //}
    }
};

BulletDebugDrawer::BulletDebugDrawer() {
    m_LinePoints.reserve(2048);
    m_LinePointsColors.reserve(2048);   
}

BulletDebugDrawer::~BulletDebugDrawer() {}

void BulletDebugDrawer::Prepare() {
    m_LinePoints.clear();
    m_LinePointsColors.clear();
}

void BulletDebugDrawer::Submit(const MoonGlare::Core::MoveConfig &conf) {
    auto frame = conf.m_BufferFrame;

    if (!ready) {
        m_LinePoints.clear();
        m_LinePointsColors.clear();

        auto &shres = frame->GetResourceManager()->GetShaderResource();
        if (!shres.Load<BtDebugDrawShaderDescriptor>(shaderHandle, "btDebugDraw")) {
            AddLogf(Error, "Failed to load GUI shader");
            return;
        }

        ready = true;
        return;
    }
    using namespace Renderer;

    Renderer::VAOResourceHandle vao;
    if (!frame->AllocateFrameResource(vao))
        return;

//    auto &mem = frame->GetMemory();

    auto &layers = frame->GetCommandLayers();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::DefferedLighting>();
    auto &q = Queue;

    auto &shres = frame->GetResourceManager()->GetShaderResource();
    auto shb = shres.GetBuilder<BtDebugDrawShaderDescriptor>(q, shaderHandle);

    shb.Bind();
    using Uniform = BtDebugDrawShaderDescriptor::Uniform;
    shb.Set<Uniform::CameraMatrix>(conf.deferredSink->m_Camera.GetProjectionMatrix());

    auto &m = frame->GetMemory();
    using ichannels = Renderer::Configuration::VAO::InputChannels;

    auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vao, true);
    vaob.BeginDataChange();

    vaob.CreateChannel(ichannels::Vertex);
    vaob.SetChannelData<float, 3>(ichannels::Vertex, 
        (const float*)m.Clone(m_LinePoints), m_LinePoints.size());

    vaob.CreateChannel(ichannels::Texture0);
    vaob.SetChannelData<float, 3>(ichannels::Texture0, 
        (const float*)m.Clone(m_LinePointsColors), m_LinePointsColors.size());

    vaob.EndDataChange();

    q.MakeCommand<Commands::DepthMask>((GLboolean)GL_FALSE);
    q.MakeCommand<Commands::Enable>((GLenum)GL_DEPTH_TEST);
    q.MakeCommand<Commands::Disable>((GLenum)GL_BLEND);
    q.MakeCommand<Commands::Disable>((GLenum)GL_CULL_FACE);

    auto arg = Queue.PushCommand<Renderer::Commands::VAODrawArrays>();
    arg->mode = GL_LINES;
    arg->first = 0;
    arg->count = m_LinePoints.size();

    vaob.UnBindVAO();
}

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
    m_LinePoints.push_back(convert(from));
    m_LinePointsColors.push_back(convert(color));

    m_LinePoints.push_back(convert(to));
    m_LinePointsColors.push_back(convert(color));
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
//	m_dev->CurrentShader()->SetBackColor(convert(color));
    //glBegin(GL_POINTS);
    //glVertex3fv((float*)&PointOnB);
    //glEnd();
    //glBegin(GL_LINES);
    //auto sum = PointOnB + normalOnB;
    //glVertex3fv((float*)&PointOnB);
    //glVertex3fv((float*)&sum);
    //glEnd();
}

void BulletDebugDrawer::reportErrorWarning(const char *c) {
    AddLog(Warning, "Message from bullet: '" << c << "'");
}

void BulletDebugDrawer::draw3dText(const btVector3 &, const char *) {}

void BulletDebugDrawer::setDebugMode(int p) {
}

int BulletDebugDrawer::getDebugMode(void) const {
    return 
        DBG_DrawWireframe | 
        DBG_DrawAabb | 
//        DBG_DrawNormals | 
        0;
}

} //namespace Physics 
