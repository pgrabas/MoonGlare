/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:58,54
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "ResourceManager.h"
#include "../Renderer.h"

#include "AsyncLoader.h"
#include "MeshResource.h"
#include "MaterialManager.h"

namespace MoonGlare::Renderer::Resources {

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

iAsyncLoader* ResourceManager::GetLoader() {
    return m_AsyncLoader.get();
}

bool ResourceManager::Initialize(RendererFacade *Renderer, iFileSystem *fileSystem) {
    RendererAssert(Renderer);
    RendererAssert(fileSystem);

    m_RendererFacade = Renderer;

    m_AsyncLoader = std::make_unique<AsyncLoader>(this, fileSystem, m_RendererFacade->GetConfiguration());

//    auto conf = m_RendererFacade->GetConfiguration();

    textureResource = mem::make_aligned<TextureResource>();
    textureResource->Initialize(this, fileSystem);

    vaoManager = mem::make_aligned<VAOResource>(this);

    if (!m_ShaderResource.Initialize(this, fileSystem)) {
        AddLogf(Error, "ShaderResource initialization failed!");
        return true;
    }

    materialManager = mem::make_aligned<MaterialManager>(this);
    meshManager = mem::make_aligned<MeshManager>(this);


    return true;
}

bool ResourceManager::Finalize() {
    m_AsyncLoader.reset();

    vaoManager.reset();
    materialManager.reset();

    if (!m_ShaderResource.Finalize()) {
        AddLogf(Error, "ShaderResource finalization failed!");
    }

    textureResource->Finalize();
    textureResource.reset();

    return true;
}

const Configuration::RuntimeConfiguration * ResourceManager::GetConfiguration() const {
    return m_RendererFacade->GetConfiguration();
}

} //namespace MoonGlare::Renderer::Resources 
